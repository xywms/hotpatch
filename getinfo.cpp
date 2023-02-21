#include<iostream>
#include<string>
#include<stdlib.h>
#include<vector>
#include<set>
#include<map>
#include <fstream>
#include <sstream>
#include <cstdlib>
using namespace std;
//popen运行命令行
string cmdopen(string cmd) {
    char line[1000];
    FILE *fp;
    // system call
    const char *sysCommand = cmd.data();
    if ((fp = popen(sysCommand, "r")) == NULL) {
        cout << "error" << endl;
    }
    while (fgets(line, sizeof(line)-1, fp) != NULL){
        cout << line << endl;
    }
    pclose(fp);
    return line;
}
//寻找函数地址
vector<string> findfuncentry(set<string> funcnameset,string kernelpath) {
    //在system.map中寻找函数地址
    vector<string>funcaddrvec;
    for(set<string>::iterator it = funcnameset.begin();it!=funcnameset.end();it++) {
        //使用cmd命令
        string funcaddr;
        funcaddr = cmdopen("cd " + kernelpath + "&&" + "grep -w " + *it + " System.map ");
        if(funcaddr.size() != 0) {
            funcaddrvec.push_back(funcaddr.substr(0,16));
        }else {
            cout<<"cannot find the function in System.map！";
        }
    }
    return funcaddrvec;
}
//识别函数列表
set<string> regfunclist(vector<string> patchv) {
    set<string> funclist;
    for(int i = 0; i < patchv.size(); i++) {
        if(patchv[i].find("@@") != patchv[i].npos) {
            //如果该行存在(，则为函数修改
            if(patchv[i].find("(") != patchv[i].npos) {
                //首先截取@之后，(之前的字符串
                string funcname = patchv[i].substr( patchv[i].find_last_of("@") + 2,  patchv[i].find_first_of("(") -  patchv[i].find_last_of("@") - 2);
                funcname = funcname.substr(funcname.find_last_of(" ") + 1,funcname.size() - funcname.find_last_of(" ") );
                funclist.insert(funcname);
            }else {
                cout<<"it is not a function change!"<<endl;
                continue;
            }
        }
    }
    return funclist;
}
//编写脚本文件
void makeshfile (string kernelcodepath, string arm_linux_path) {
    //脚本文件
    ofstream fcompile;
    fcompile.open(kernelcodepath + "/testmake.sh", ios::out);
     //文件写入
	fcompile<<"#!/bin/bash"<<endl;
    fcompile<<"cd " + kernelcodepath <<endl;
    fcompile<<"sudo make mrproper"<<endl;
    fcompile<<"sudo make clean"<<endl;
    fcompile<<"sudo cp -v /boot/config-$(uname -r) .config"<<endl;
    fcompile<<"sudo scripts/config -d CONFIG_X86_X32"<<endl;
    fcompile<<"sudo scripts/config -d SYSTEM_TRUSTED_KEYS"<<endl;
    fcompile<<"sudo scripts/config -d SYSTEM_REVOCATION_KEYS"<<endl;

    fcompile<<"echo | sudo make ARCH=\"arm64\" CROSS_COMPILE=" + arm_linux_path + " config"<<endl;
    fcompile<<"echo | sudo make ARCH=\"arm64\" CROSS_COMPILE=" + arm_linux_path + " -j8"<<endl;
    //fcompile<<"aarch64-linux-gnu-objdump -D vmlinux > vmlinux.out"<<endl;
  
    // fcompile<<"symbol=$1"<<endl;
    // fcompile<<"startaddress=$(nm -n vmlinux | grep -w \"\\w\\s$symbol\" | awk '{print \"0x\"$1;exit}') "<<endl;
    // fcompile<<"endaddress=$(nm -n vmlinux | grep -w -A1 \"\\w\\s$symbol\" | awk '{getline; print \"0x\"$1;exit}')"<<endl;
    // fcompile<<"echo \"start-address: $startaddress, end-address: $endaddress\""<<endl;
    // fcompile<<"aarch64-linux-gnu-objdump -d vmlinux --start-address=$startaddress --stop-address=$endaddress >> asm.txt"<<endl;
    //关闭文件
    fcompile.close();
}

int main() {
    //1.读入patch文件
    fstream fpatch;
    //输入patch文件名
    cout<<"请输入patch文件的完整路径,如/home/xyw/linux/test.patch"<<endl<<":";
    string patchpath;
    cin>>patchpath;
    //截取patch文件名
    string patchname;
    patchname = patchpath.substr(patchpath.find_last_of("/") + 1, patchpath.find_last_of(".") - (patchpath.find_last_of("/")) - 1);
    fpatch.open(patchpath, ios::in);
    vector<string>patchv;
    while(!fpatch.eof()){
        string str;
        getline(fpatch, str);
        patchv.push_back(str);
    }
    fpatch.close();

    //2.识别函数列表
    set<string>funclist;
    funclist = regfunclist(patchv);
    if(funclist.size() == 0) {
       exit(1);
    }
    //测试输出
    for(set<string>::iterator it = funclist.begin();it!=funclist.end();it++) {
        cout<<*it<<endl;
    }
    //3.输出每个函数的入口地址

    //输入内核代码的路径
    string kernelcodepath;
    cout<<"请输入内核路径,如/home/xyw/linux"<<endl<<":";
    cin>>kernelcodepath;
    vector<string>funcaddrvec;
    funcaddrvec = findfuncentry(funclist, kernelcodepath);
    //测试输出
    // for(int i = 0; i < funcaddrvec.size(); i++) {
    //     cout<<funcaddrvec[i]<<endl;
    // }
    if(funcaddrvec.size() == 0) {
        exit(1);
    }
    //4.打上补丁
    cmdopen("cd "+kernelcodepath + "&&" + "patch -p1 < " + patchpath);

    //5.编译内核
    //输入交叉编译链的路径：
    cout<<"请输入交叉编译链的路径,如/home/xyw/gcc-linaro-4.9.4-2017.01-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-"<<endl<<":";
    string arm_linux_path;
    cin>>arm_linux_path;
   
    //赋予脚本权限
    makeshfile(kernelcodepath, arm_linux_path);
    cmdopen("cd " + kernelcodepath + "&&" + "chmod u+x testmake.sh");
    string sudo_password;
    cout<<"请输入sudo密码:";
    cin>>sudo_password;
    //进行编译
    //cmdopen("cd " + kernelcodepath + "&&echo " + sudo_password + " | sudo -S ./testmake.sh ");
    //遍历funclist
    int i = 0;
    for(set<string>::iterator it = funclist.begin();it!=funclist.end(), i < funcaddrvec.size();it++, i++) {
        //输出函数名
        cmdopen("cd " + kernelcodepath + "&& echo " + *it + " >> asm.txt");
        //输出函数入口地址
        cmdopen("cd " + kernelcodepath + "&& echo " + funcaddrvec[i] + " >> asm.txt");
        //输出函数汇编码

        // string startaddress = cmdopen("cd "+ kernelcodepath + "&& nm -n vmlinux | grep -w \"\\w\\s"+ *it + "\" | awk '{print \"0x\"$1;exit}'");
        // string endaddress = cmdopen("cd "+ kernelcodepath + "&& nm -n vmlinux | grep -w -A1 \"\\w\\s"+ *it + "\" | awk '{getline; print \"0x\"$1;exit}'");
        // cout<<"startaddress = "<<startaddress<<",endaddress = "<<endaddress;
        // cmdopen("cd "+ kernelcodepath + "&& aarch64-linux-gnu-objdump -d vmlinux --start-address="+ startaddress + " --stop-address=" + endaddress + " >> asm.txt");
        //清空文件
        ofstream fcompile;
        fcompile.open(kernelcodepath + "/testout.sh", ios::out);
        fcompile<<"#!/bin/bash"<<endl;
        fcompile<<"cd " + kernelcodepath <<endl;
        fcompile<<"symbol=$1"<<endl;
        fcompile<<"startaddress=$(nm -n vmlinux | grep -w \"\\w\\s$symbol\" | awk '{print \"0x\"$1;exit}') "<<endl;
        fcompile<<"endaddress=$(nm -n vmlinux | grep -w -A1 \"\\w\\s$symbol\" | awk '{getline; print \"0x\"$1;exit}')"<<endl;
        fcompile<<"echo \"start-address: $startaddress, end-address: $endaddress\""<<endl;
        fcompile<<"aarch64-linux-gnu-objdump -d vmlinux --start-address=$startaddress --stop-address=$endaddress >> asm.txt"<<endl;
        fcompile.close();
        cmdopen("cd " + kernelcodepath + "&&" + "chmod u+x testout.sh");
        cmdopen("cd " + kernelcodepath + "&&" + " ./testout.sh " + *it);
    }
    

}
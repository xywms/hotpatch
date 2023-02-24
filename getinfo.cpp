#include<iostream>
#include<string>
#include<stdlib.h>
#include<vector>
#include<set>
#include<map>
#include <fstream>
#include <sstream>
#include <cstdlib>
typedef int ul_32;
typedef long long ll_64;
using namespace std;
//bl指令的数量
int bl_count = 0;
int bl_count_all = 0;
//1个bl指令需要增加的偏移
ul_32 bl_off = 0x4 * 5;

//adrp的指令地址按ffff ff80 1200 0000来算
//只需要考虑跳转到函数外的adrp,30:29 immlo 23:5 immhi,偏移为immhi:immlo
ll_64 adrp_addr = 0xffffff8012000000;
//计算addr_jmp

ll_64 cal_adrp_jmp(ll_64 addr, ul_32 mcode_old) {
    ll_64 addr_jmp;
    ll_64 off_old;
    ul_32 immlo_30_29;
    ul_32 immhi_23_5;

    immlo_30_29 = (mcode_old >> 29) & 0x00000003;
    //printf("immlo_30_29:0x%x\n",immlo_30_29);
    immhi_23_5 = (mcode_old >> 5) & 0x0007ffff;
    //printf("immhi_23_5:0x%x\n",immhi_23_5);
    off_old = ((immhi_23_5 << 2) + immlo_30_29) << 12;
    //printf("off_old:0x%llx\n",off_old);
    addr_jmp = (addr & 0xffffffffffff000) + off_old;
    return addr_jmp;
}
uint32_t cal_adrp(ll_64 addr_jmp, ul_32 mcode_old) {
    ul_32 adrp_31_24;
    ul_32 immlo_30_29;
    ul_32 immhi_23_5;
    ul_32 rd_4_0;
    ul_32 off_old;
    //ul_32 off_new;
    ul_32 off_off;
    ul_32 mcode_new;
    
    //两者页面基地址之差
    ll_64 off_new = ((addr_jmp - adrp_addr) >> 12) << 12;
    //printf("off_new:0x%llx\n",off_new);
    int flag = 1;
    //如果两者页面基地址偏移是负数
    if(off_new >> 63) {
        flag = -1;
    }
    //清除30:29,23:5
    immlo_30_29 = (off_new << (29 - 12)) & 0x60000000;
    immhi_23_5 = (off_new >> (14 - 5)) & 0x00ffffe0;

    mcode_new = mcode_old & 0x9f00001f + immlo_30_29  + immhi_23_5;
    return mcode_new;

}
ll_64 cal_b_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 b_31_26;
    ul_32 imm19_25_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    imm19_25_0 = mcode_old & 0x3ffffff;
    //如果是负数，右移高位补1
    int flag = 1;
    if(imm19_25_0 >> 25) {
        flag = -1;
        imm19_25_0 = 0xfc000000 + imm19_25_0;
        // printf(":0x%x\n",imm19_25_0);
    }
    //times4即左移两位
    off_old = imm19_25_0 << 2;
    //printf("off_old:0x%x\n",off_old);
    //计算跳转地址
    ll_64 b_addr_jmp = addr + (ll_64) off_old;
    printf("b_addr_jmp:0x%llx\n",b_addr_jmp);
    return b_addr_jmp;
}
uint32_t cal_b(ll_64 addr, ul_32 mcode_old) {
    ul_32 b_31_26;
    ul_32 imm19_25_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    
    imm19_25_0 = mcode_old & 0x3ffffff;
    //如果是负数，右移高位补1
    int flag = 1;
    if(imm19_25_0 >> 25) {
        flag = -1;
        imm19_25_0 = 0xfc000000 + imm19_25_0;
        // printf(":0x%x\n",imm19_25_0);
    }
    //times4即左移两位
    off_old = imm19_25_0 << 2;
    //printf("off_old:0x%x\n",off_old);
   
    //加上增加的偏移得到新偏移
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    //方法一
    //将旧机器码的所有部分分开，再合起来
    // b_31_26 = 0x5;//000101
    // printf(":0x%x\n",b_31_26<<26);
    // printf(":0x%x\n",off_new>>2);
    // //右移如何使高位加1,而且现在负数偏移的加减有问题
    // mcode_new = (b_31_26<<26) + ((off_new >> 2) & 0x03ffffff) ;

    //方法二
    //将25：0清零
    mcode_new = (mcode_old & 0xfc000000) + ((off_new >> 2) & 0x03ffffff);
    return mcode_new;

}

ll_64 cal_beq_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 beq_31_24;
    ul_32 imm19_23_5;
    ul_32 cond_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;
    
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xfff8000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
     //计算跳转地址
    ll_64 beq_addr_jmp = addr + (ll_64) off_old;
    printf("beq_addr_jmp:0x%llx\n",beq_addr_jmp);
    return beq_addr_jmp;
}
uint32_t cal_beq(ll_64 addr, ul_32 mcode_old) {
    ul_32 beq_31_24;
    ul_32 imm19_23_5;
    ul_32 cond_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;
    
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xfff8000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
   
    //加上增加的偏移得到新偏移
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    //方法一：
    //将旧机器码的所有部分分开，再合起来
    //beq_31_24 = 0x54;//0101 0100
    //cond_4_0 = mcode_old & 0x1f;
    //mcode_new = (beq_31_24<<24) + (off_new << 3) + cond_4_0;
    //方法二：将23：5清零，再加上imm19,off_new右移两位，再左移5位
    mcode_new = (mcode_old & 0xff00001f) + (off_new << 3) ;
    return mcode_new;
}
ll_64 cal_cbz_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 cbz_31_24;
    ul_32 imm19_23_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    //截取原机器码的23:5，再右移两位，得到原偏移
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xff80000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    //计算跳转地址
    ll_64 cbz_addr_jmp = addr + (ll_64) off_old;
    printf("cbz_addr_jmp:0x%llx\n",cbz_addr_jmp);
    return cbz_addr_jmp;
}
uint32_t cal_cbz(ll_64 addr, ul_32 mcode_old) {
    
    ul_32 cbz_31_24;
    ul_32 imm19_23_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    //截取原机器码的23:5，再右移两位，得到原偏移
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xff80000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
   
    //加上增加的偏移得到新偏移
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    //将旧机器码的所有部分分开，再合起来
    //cbz_31_24 = 0xb4;//10110100
    //rt_4_0 = mcode_old & 0x1f;
    //mcode_new = (cbz_31_24<<24) + (off_new << 3) + rt_4_0;
    //或者先给23：5清零，再加上imm19
    mcode_new = (mcode_old & 0xff00001f) + (off_new << 3);
    return mcode_new;

}
ll_64 cal_cbnz_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 cbnz_31_24;
    ul_32 imm19_23_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;
   
    //截取原机器码的23:5
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xff80000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    //计算跳转地址
    ll_64 cbnz_addr_jmp = addr + (ll_64) off_old;
    printf("cbnz_addr_jmp:0x%llx\n",cbnz_addr_jmp);
    return cbnz_addr_jmp;
}
uint32_t cal_cbnz(ll_64 addr, ul_32 mcode_old) {
    ul_32 cbnz_31_24;
    ul_32 imm19_23_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;
   
    //截取原机器码的23:5
    imm19_23_5 = (mcode_old >> 5) & 0x7ffff;
    int flag = 1;
    if(imm19_23_5 >> 18) {
        flag = -1;
        imm19_23_5 = 0xff80000 + imm19_23_5;
    }
    //times4即左移两位
    off_old = imm19_23_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    //加上增加的偏移得到新偏移
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    //将旧机器码的所有部分分开，再合起来
    // cbnz_31_24 = 0xb5;//10110101
    // rt_4_0 = mcode_old & 0x1f;
    //mcode_new = (cbnz_31_24<<24) + (off_new << 3) + rt_4_0;
    mcode_new = (mcode_old & 0xff00001f) + (off_new << 3);
    return mcode_new;
}

ll_64 cal_tbz_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 tbz_31_24;
    ul_32 imm14_18_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    imm14_18_5 = (mcode_old >> 5) & 0x3fff;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    int flag = 1;
    if(imm14_18_5 >> 13) {
        flag = -1;
        imm14_18_5 = 0xfffc0000 + imm14_18_5; 
    }

    off_old = imm14_18_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    //计算跳转地址
    ll_64 tbz_addr_jmp = addr + (ll_64) off_old;
    printf("tbz_addr_jmp:0x%llx\n",tbz_addr_jmp);
    return tbz_addr_jmp;
}
uint32_t cal_tbz(ll_64 addr, ul_32 mcode_old) {
    ul_32 tbz_31_24;
    ul_32 imm14_18_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    imm14_18_5 = (mcode_old >> 5) & 0x3fff;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    int flag = 1;
    if(imm14_18_5 >> 13) {
        flag = -1;
        imm14_18_5 = 0xfffc0000 + imm14_18_5; 
    }

    off_old = imm14_18_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    imm14_18_5 = (off_new << (5-2)) & 0x0007ffe0;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    mcode_new = (mcode_old & 0xfff9001f) + imm14_18_5;
    return mcode_new;
}

ll_64 cal_tbnz_addr_jmp(ll_64 addr, ul_32 mcode_old) {
    ul_32 tbnz_31_24;
    ul_32 imm14_18_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    imm14_18_5 = (mcode_old >> 5) & 0x3fff;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    int flag = 1;
    if(imm14_18_5 >> 13) {
        flag = -1;
        imm14_18_5 = 0xfffc0000 + imm14_18_5; 
    }

    off_old = imm14_18_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    //计算跳转地址
    ll_64 tbnz_addr_jmp = addr + (ll_64) off_old;
    printf("tbnz_addr_jmp:0x%llx\n",tbnz_addr_jmp);
    return tbnz_addr_jmp;
}
uint32_t cal_tbnz(ll_64 addr, ul_32 mcode_old) {
    ul_32 tbnz_31_24;
    ul_32 imm14_18_5;
    ul_32 rt_4_0;
    ul_32 off_old;
    ul_32 off_new;
    ul_32 mcode_new;

    imm14_18_5 = (mcode_old >> 5) & 0x3fff;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    int flag = 1;
    if(imm14_18_5 >> 13) {
        flag = -1;
        imm14_18_5 = 0xfffc0000 + imm14_18_5; 
    }

    off_old = imm14_18_5 << 2;
    //printf("off_old:0x%x\n",off_old);
    
    off_new = off_old + bl_count * bl_off * flag;
    //printf("off_new:0x%x\n",off_new);
    imm14_18_5 = (off_new << (5-2)) & 0x0007ffe0;
    //printf("imm14_18_5:0x%x\n",imm14_18_5);
    mcode_new = (mcode_old & 0xfff9001f) + imm14_18_5;
    return mcode_new;
}
//bl的跳转指令
ll_64 cal_bl_jmp(ll_64 addr, ul_32 mcode_old) {
    ll_64 bl_jmp;
    ul_32 imm26_25_0;
    ll_64 off_old;
    imm26_25_0 = mcode_old & 0x03ffffff;
    off_old = imm26_25_0 << 2;
    bl_jmp = addr + off_old;
    return bl_jmp;
}
//立即数取跳转地址的31：16
uint32_t cal_movk_16(ll_64 addr_jmp) {
    //movk x4, #0x 31:16,lsl #16 
    ul_32 imm16_20_5;
    ul_32 rd_4_0 = 0x4;
    ul_32 mcode_new;
    imm16_20_5 = (addr_jmp >> 16) & 0x0000ffff;
    //hw = 01
    mcode_new = 0xf2a00000 + (imm16_20_5 << 5) + rd_4_0;
    return mcode_new;
    
}
//立即数取跳转地址的47：32
uint32_t cal_movk_32(ll_64 addr_jmp) {
    //movk x4, #0x 47:32,lsl #32 
    ul_32 imm16_20_5;
    ul_32 rd_4_0 = 0x4;
    ul_32 mcode_new;
    //printf("addr_jmp:0x%llx\n",addr_jmp);
    imm16_20_5 = (addr_jmp >> 32) & 0x0000ffff;
    //printf("imm16_20_5:0x%x\n",imm16_20_5);
    //hw = 10
    mcode_new = 0xf2c00000 + (imm16_20_5 << 5) + rd_4_0;
    return mcode_new;
}
uint32_t cal_mov(ll_64 addr, ul_32 mcode_old, ll_64 bl_addr_jmp) {
    
    //首先根据bl指令地址和机器码算出jmp地址
    //bl指令的低26位为imm26
    
    //bl_addr_jmp = addr + (ll_64)((mcode_old << 2) & 0x000000000fffffff);
    //printf("addr_jmp:0x%llx\n",bl_addr_jmp);
    //再根据jmp地址计算三条指令，一条mov,两条movk
    //mov的跳转地址为0xffff ffff ffff xxxx
    ul_32 imm16_20_5;
    //表示寄存器
    ul_32 rd_4_0 = 0x4;
    ul_32 mcode_new;
    //取低16位
    imm16_20_5 = bl_addr_jmp & 0x0000ffff;
    //printf("imm16_20_5:0x%x\n",imm16_20_5);
    //将imm16_20_5取反,并将前12位清零
    imm16_20_5 = (~ imm16_20_5) & 0x0000ffff;
    //printf("imm16_20_5:0x%x\n",imm16_20_5);
    mcode_new = 0x92800000 + (imm16_20_5 << 5) + rd_4_0;
    return mcode_new;

}
uint32_t cal_blr(ul_32 rn) { 
    ul_32 rn_9_5;
    rn_9_5 = rn << 5;
    ul_32 mcode_new = 0xd63f0000 + rn_9_5;
    //printf("mcode_new:0x%x\n",mcode_new);
    return mcode_new;
}
//参考手册str(immediate ->unsiged offset)
// str xn, [sp]
uint32_t cal_str(ul_32 rt) {
    ul_32 imm12_21_10;
    //sp相当于x31
    ul_32 rn_9_5 = 0x3e0;
    //sp的偏移
    //当立即数为32时
    //ul_32 str_off = 0x20;
    //当立即数为0时
    ul_32 str_off = 0x0;
    //str_off = imm12 times 8
    imm12_21_10 = str_off << (10 - 3);
    //printf("imm12_21_10:0x%x\n",imm12_21_10);
    ul_32 mcode_new = 0xf9000000 + imm12_21_10 + rn_9_5 + rt;
    return mcode_new;
}
//ldr xn,[sp]
uint32_t cal_ldr(ul_32 rt) {
    ul_32 imm12_21_10;
    //当右侧寄存器为x21时
    //ul_32 rn_9_5  = 0x2a0;
    //当右侧为sp时，sp相当于x31
    ul_32 rn_9_5 = 0x3e0;
    ul_32 ldr_off = 0x0;
     //str_off = imm12 times 8
    imm12_21_10 = ldr_off << (10 - 3);

    ul_32 mcode_new = 0xf9400000 + imm12_21_10 + rn_9_5 + rt;
    return mcode_new;
}
//建立指令地址与索引的映射
map<ll_64,int>m_addr_instruc;

int cal_bl_count(ll_64 addr_start, ll_64 addr_end,vector<int> bl_id_s) {
    int start = m_addr_instruc[addr_start];
    int end = m_addr_instruc[addr_end];
    cout<<start<<","<<end<<",";
    if(start > end) {
        int temp = start;
        start = end;
        end = temp;
    }
    cout<<bl_id_s[end] - bl_id_s[start]<<",";
    return bl_id_s[end] - bl_id_s[start];

}
int mcode_startid = 0;
int instruct_startid = 0;
//第一条有效指令的位置
int all_start = 1;
//通过机器码判断指令
string mcode_instruc(ul_32 mcode) {
    //提取30：26
    ul_32 mcode_31_26 = (mcode >> 26) & 0x0000003f ;
    //b  31：26 000101
    if(mcode_31_26 == 0x5) {
       return "b";
    //bl 31:26 100101
    }else if(mcode_31_26 == 0x25) {
        return "bl";
    }
    //b.cond 31:24 01010100
    ul_32 mcode_31_24 = (mcode >> 24) & 0x000000ff;
    if(mcode_31_24 == 0x54) {
        return "b.cond";
    }
    //cbz 31:24 10110100
    else if(mcode_31_24 == 0xb4) {
        return "cbz";
    }
    //cbnz 31:24 10110101
    else if(mcode_31_24 == 0xb5) {
        return "cbnz";
    }
    //tbz 30:24 0110110
    ul_32 mcode_30_24 = (mcode >> 24) & 0x0000007f;
    if(mcode_30_24 == 0x36) {
        return "tbz";
    }
    //tbnz 30:24 0110111
    if(mcode_30_24 == 0x37) {
        return "tbnz";
    }
    //adrp  31 1  28：24 10000
    ul_32 mcode31_28_24 = (mcode >> 24) & 0x0000009f;
    if(mcode31_28_24 == 0x90) {
        return "adrp";
    }
    return "other instructions";
}
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
// set<string> regfunclist(vector<string> patchv) {
//     set<string> funclist;
//     for(int i = 0; i < patchv.size(); i++) {
//         if(patchv[i].find("@@") != patchv[i].npos) {
//             //如果该行存在(，则为函数修改
//             if(patchv[i].find("(") != patchv[i].npos) {
//                 //首先截取@之后，(之前的字符串
//                 string funcname = patchv[i].substr( patchv[i].find_last_of("@") + 2,  patchv[i].find_first_of("(") -  patchv[i].find_last_of("@") - 2);
//                 funcname = funcname.substr(funcname.find_last_of(" ") + 1,funcname.size() - funcname.find_last_of(" ") );
//                 funclist.insert(funcname);
//             //修改变量
//             }else {
//                 continue;
//             }
//         }
//     }
//     return funclist;
// }
string regfunclist(string theline) {
    string funcname;
    funcname = theline.substr(theline.find_last_of("@") + 2,  theline.find_first_of("(") -  theline.find_last_of("@") - 2);
    funcname = funcname.substr(funcname.find_last_of(" ") + 1,funcname.size() - funcname.find_last_of(" ") );
    return funcname;
}
//识别全局变量
string regvarlist(vector<string> patchv, int i) {
    string varname;
    for(int k = i + 1; k < patchv.size(); k++) {
        //判断该行是否修改了变量
        if(patchv[k].find("@@") != patchv[k].npos) {
            cout<<"已到下一个修改";
            break;
        }
        string theline = patchv[k];
        if(theline[0] == '-' || theline[0] == '+') {
            //如果存在等号,变量名即为等号前最后一个字符串
            if(theline.find("=") != theline.npos) {
                varname = theline.substr(theline.find_first_of(" ") + 1, theline.find_last_of("=") - theline.find_first_of(" ") - 2);
                if(varname.find(" ") != varname.npos) {
                    varname = varname.substr(varname.find_last_of(" "),varname.size() - varname.find_last_of(" "));
                }
                return varname;
            }else {
                //如果没有等号，变量名则为分号前的最后一个字符串
                varname = theline.substr(theline.find_first_of(" ") + 1,theline.find_last_of(";") - theline.find_first_of(" ") - 1);
                if(varname.find(" ") != varname.npos) {
                    varname = varname.substr(varname.find_last_of(" "),varname.size() - varname.find_last_of(" "));
                }
                return varname;
            }
        }
    }
    return varname;
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
    //fcompile<<"sudo scripts/config -d CONFIG_X86_X32"<<endl;
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

    //2.识别修改内容
    set<string>funclist;
    set<string>varlist;
    //funclist = regfunclist(patchv);
    for(int i = 0; i < patchv.size(); i++) {
        if(patchv[i].find("@@") != patchv[i].npos) {
            //如果该行存在(且无分号则为函数修改
            if(patchv[i].find("(") != patchv[i].npos && patchv[i].find(";") == patchv[i].npos) {
                string funcname = regfunclist(patchv[i]);
                if(funcname.size() != 0) {
                    funclist.insert(funcname);
                }
            //否则为变量修改
            }else {
                string varname = regvarlist(patchv, i);
                if(varname.size() != 0) {
                    varlist.insert(varname);
                }
            }
        }else {
            continue;
        }
    }
    if(funclist.size() == 0 && varlist.size() == 0) {
       exit(1);
    }
    //测试输出
    for(set<string>::iterator it = funclist.begin();it!=funclist.end();it++) {
        cout<<*it<<endl;
    }

    //3.输入内核代码的路径
    string kernelcodepath;
    cout<<"请输入内核路径,如/home/xyw/linux"<<endl<<":";
    cin>>kernelcodepath;

    //测试输出
    // for(int i = 0; i < funcaddrvec.size(); i++) {
    //     cout<<funcaddrvec[i]<<endl;
    // }
    //4.打上补丁
    cmdopen("cd "+kernelcodepath + "&&" + "patch -p1 < " + patchpath);

    //5.编译内核
    //输入交叉编译链的路径：
    cout<<"请输入交叉编译链的路径,如/home/xyw/gcc-linaro-6.2.1-2016.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-"<<endl<<":";
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
    for(set<string>::iterator it = funclist.begin();it != funclist.end(); it++) {
        //输出函数汇编码
        // string startaddress = cmdopen("cd "+ kernelcodepath + "&& nm -n vmlinux | grep -w \"\\w\\s"+ *it + "\" | awk '{print \"0x\"$1;exit}'");
        // string endaddress = cmdopen("cd "+ kernelcodepath + "&& nm -n vmlinux | grep -w -A1 \"\\w\\s"+ *it + "\" | awk '{getline; print \"0x\"$1;exit}'");
        // cout<<"startaddress = "<<startaddress<<",endaddress = "<<endaddress;
        // cmdopen("cd "+ kernelcodepath + "&& aarch64-linux-gnu-objdump -d vmlinux --start-address="+ startaddress + " --stop-address=" + endaddress + " >> asm.txt");
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

        //输出函数名
        cmdopen("cd " + kernelcodepath + "&& echo " + *it + " > result.txt");
        //输出函数入口地址
        string funcaddr = cmdopen("cd " + kernelcodepath + "&&" + "grep -w " + *it + " System.map ");
        cmdopen("cd " + kernelcodepath + "&& echo " + funcaddr.substr(0, 16) + " >> result.txt");


    //     //计算机器码,写入result文件
    //     ifstream fp;
    //     //将所有字符串保存在vector中
    //     vector<string>v;
    //     int count = 0;
    //     int i = 0;
    //     //指定打开方式
    //     fp.open("asm.txt", ios::in);
    //     //按行读入字符串
    //     while(!fp.eof()){
    //         string str;
    //         getline(fp, str);
    //         //去除空格行
    //         if(str != "" ) {
    //             if(str.substr(0,1) == "f")
    //             {
    //                 count++;
    //                 v.push_back(str);
    //             }
            
    //         }
    //     }
    //     //去掉末尾下一个函数的干扰
    //     if(v[count - 1].substr(16,1) != ":") {
    //         v.pop_back();
    //         count--;
    //     }
    //     vector<ll_64>addr(count);
    //     vector<ul_32>mcode(count);
    //     //记录bl指令的索引
    //     vector<int>bl_id(count);
    //     //bl_id的前缀和
    //     vector<int>bl_id_s(count);
    //     //初始化bl_id
    //     for(int i = all_start; i < count ; i++) {
    //         bl_id[i] = 0;
    //     }
    
    //     //寻找mcode的开始位置
    //     for(int i = 18; i < 25; i++) {
    //         if(v[1][i] != ' ') {
    //             mcode_startid = i;
    //             break;
    //         } 
    //     }
    //     cout<<"mcode_startid:"<<mcode_startid<<endl;
    //     //寻找指令字符的开始位置
    //     for(int i = mcode_startid + 8; i < 35; i++) {
    //         if(v[1][i] != ' ') {
    //             instruct_startid = i;
    //             break;
    //         } 
    //     }
    //     cout<<"instruct_startid:"<<instruct_startid<<endl;
    //     //记录bl指令的位置
    //     for(int i = all_start; i < count; i++) {
    //         addr[i] = strtoull(v[i].substr(0,16).c_str(), NULL, 16);
    //         mcode[i] = strtoul(v[i].substr(mcode_startid,8).c_str(), NULL, 16);
    //         //printf("mcode[i]:0x%x\n",mcode[i]);
    //         m_addr_instruc.insert({addr[i],i});
    //         if(mcode_instruc(mcode[i]) == "bl") {
    //             cout<<i<<"bl_id = 1"<<endl;
    //             bl_id[i] = 1;
    //             bl_count_all++;
    //         }
    //     }
    //     //输入开始与结束索引，计算两者之间的1的数量，用一维前缀和
    
    //     for(int i = all_start + 1; i < count ; i++) {
    //         bl_id_s[i] = bl_id_s[i - 1] + bl_id[i];
    //     }
    // //重新计算与替换其他跳转指令
    //     //--------------------输出到文件
    //     ofstream fs;
    //     fs.open("result.txt", ios::out);
    //     //文件写入
    //     fs<<"uint32_t inst_list[";
    //     //机器码的总数量
    //     fs<<(count -1 + bl_count_all * 5);
    //     fs<<"] = {";
    //     for(int i = all_start; i < count ; i++) {
    //         if(mcode_instruc(mcode[i]) == "bl") {
    //             cout<<i<<" is bl"<<endl;
    //             //查询addr_jmp的位置
    //             int addr_jmp_startid;
    //             for(int k = instruct_startid + 8;;k++) {
    //                 if(v[i][k] != ' ') {
    //                     addr_jmp_startid = k;
    //                     break;
    //                 }
    //             }
    //             //ll_64 bl_addr_jmp = strtoull(v[i].substr(addr_jmp_startid, 16).c_str(), NULL, 16);
    //             ll_64 bl_addr_jmp = cal_bl_jmp(addr[i], mcode[i]);
    //             // printf("cal_str:0x%x\n",cal_str(0x4));
    //             ul_32 res_cal_str = cal_str(0x4);
    //             ul_32 res_cal_mov = cal_mov(addr[i], mcode[i],bl_addr_jmp);
    //             ul_32 res_cal_movk_16 = cal_movk_16(bl_addr_jmp);
    //             ul_32 res_cal_movk_32 = cal_movk_32(bl_addr_jmp);
    //             ul_32 res_cal_blr = cal_blr(0x4);
    //             ul_32 res_cal_ldr = cal_ldr(0x4);
    //             //需要转换
    //             fs<<"0x"<<hex<<res_cal_str<<",";
    //             fs<<"0x"<<hex<<res_cal_mov<<",";
    //             fs<<"0x"<<hex<<res_cal_movk_16<<",";
    //             fs<<"0x"<<hex<<res_cal_movk_32<<",";
    //             fs<<"0x"<<hex<<res_cal_blr<<",";
    //             fs<<"0x"<<hex<<res_cal_ldr;
    //             //printf("cal_str:0x%x\n",cal_str(0x4));
    //             printf("cal_mov:0x%x\n",res_cal_mov);
    //             printf("cal_movk_16:0x%x\n",res_cal_movk_16);
    //             printf("cal_movk_32:0x%x\n",res_cal_movk_32);
    //             printf("cal_blr:0x%x\n",res_cal_blr);
    //             //printf("cal_ldr:0x%x\n",cal_ldr(0x4));
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "adrp") {
    //             cout<<i<<" is adrp"<<endl;
    //             //ll_64 adrp_addr_jmp = strtoull(v[i].substr(v[i].find(",") + 2,16).c_str(), NULL, 16);
    //             //ffffff8010634738:    90ffffe0     adrp    x0, ffffff8010630000
    //             ll_64 adrp_addr_jmp = cal_adrp_jmp(addr[i], mcode[i]); 
    //             //printf("adrp_addr_jmp:0x%llx\n",adrp_addr_jmp);
    //             ul_32 res_cal_adrp = cal_adrp(adrp_addr_jmp,mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_adrp;
    //             printf("cal_adrp:0x%x\n",res_cal_adrp);
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "b.cond") {
    //             cout<<i<<" is b.cond"<<endl;
    //             //计算bl_count
    //             //  //查询addr_jmp的位置
    //             // int addr_jmp_startid;
    //             // for(int k = instruct_startid + 8;;k++) {
    //             //     if(v[i][k] != ' ') {
    //             //         addr_jmp_startid = k;
    //             //         break;
    //             //     }
    //             // }
    //             ll_64 bcond_addr_jmp = cal_beq_addr_jmp(addr[i], mcode[i]);
    //             //printf("bcond_addr_jmp:0x%llx\n",bcond_addr_jmp);
    //             bl_count = cal_bl_count(addr[i], bcond_addr_jmp,bl_id_s);
    //             //cout<<bl_count;
    //             ul_32 res_cal_beq = cal_beq(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_beq;
    //             printf("cal_beq:0x%x\n",res_cal_beq);
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "b") {
    //             cout<<i<<" is b"<<endl;
    //             //查询addr_jmp的位置
    //             // int addr_jmp_startid;
    //             // for(int k = instruct_startid + 8;;k++) {
    //             //     if(v[i][k] != ' ') {
    //             //         addr_jmp_startid = k;
    //             //         break;
    //             //     }
    //             // }
    //             //
    //             ll_64 b_addr_jmp = cal_b_addr_jmp(addr[i], mcode[i]);
    //             bl_count = cal_bl_count(addr[i], b_addr_jmp, bl_id_s);
            
    //             //cout<<"bl_count:"<<bl_count;
    //             ul_32 res_cal_b = cal_b(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_b;
    //             printf("cal_b:0x%x\n",res_cal_b);
                
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "cbz") {
    //             cout<<i<<" is cbz"<<endl;
    //             ll_64 cbz_addr_jmp = cal_cbz_addr_jmp(addr[i], mcode[i]);
    //             //printf("cbz_addr_jmp:0x%llx\n",cbz_addr_jmp);
    //             bl_count = cal_bl_count(addr[i], cbz_addr_jmp, bl_id_s);
    //             //cout<<"bl_count:"<<bl_count<<",";
    //             ul_32 res_cal_cbz = cal_cbz(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_cbz;
    //             printf("cal_cbz:0x%x\n",res_cal_cbz);
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "cbnz") {
    //             cout<<i<<" is cbnz"<<endl;
    //             ll_64 cbnz_addr_jmp = cal_cbnz_addr_jmp(addr[i], mcode[i]);
    //             //printf("cbnz_addr_jmp:0x%llx\n",cbnz_addr_jmp);
    //             bl_count = cal_bl_count(addr[i], cbnz_addr_jmp, bl_id_s);
    //             //cout<<"bl_count:"<<bl_count<<",";
    //             ul_32 res_cal_cbnz = cal_cbnz(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_cbnz;
    //             printf("cal_cbnz:0x%x\n",res_cal_cbnz);
    //             printf("\n");

    //         } 
                
    //         else if(mcode_instruc(mcode[i]) == "tbz") {
    //             cout<<i<<" is tbz"<<endl;
    //             //
    //             ll_64 tbz_addr_jmp = cal_tbz_addr_jmp(addr[i], mcode[i]);
    //             //printf("cbnz_addr_jmp:0x%llx\n",cbnz_addr_jmp);
    //             bl_count = cal_bl_count(addr[i], tbz_addr_jmp, bl_id_s);
    //             //cout<<"bl_count:"<<bl_count<<",";
    //             ul_32 res_cal_tbz = cal_tbz(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_tbz;
    //             printf("cal_tbz:0x%x\n",res_cal_tbz);
    //             printf("\n");
    //         }
    //         else if(mcode_instruc(mcode[i]) == "tbnz") {
    //             cout<<i<<" is tbnz"<<endl;
    //             ll_64 tbnz_addr_jmp = cal_tbnz_addr_jmp(addr[i], mcode[i]);
    //             //printf("tbnz_addr_jmp:0x%llx\n",tbnz_addr_jmp);
    //             bl_count = cal_bl_count(addr[i], tbnz_addr_jmp, bl_id_s);
    //             //cout<<"bl_count:"<<bl_count<<",";
    //             ul_32 res_cal_tbnz = cal_tbnz(addr[i], mcode[i]);
    //             fs<<"0x"<<hex<<res_cal_tbnz;
    //             printf("cal_tbnz:0x%x\n",res_cal_tbnz);
    //             printf("\n");
        
    //         }
    //         else {
    //             cout<<i<<":";
    //             fs<<"0x"<<hex<<mcode[i];
    //             //printf("mcode:0x%x\n",mcode[i]);
    //         }
    //         if(i != count - 1) {
    //             fs<<",";
    //         }
    //         if(i % 5 == 0) {
    //             //fs<<endl;
    //         }
    //     }
    //     fs<<"};";
    //     //还需要函数入口地址
    //     //fs<<v[all_start].substr(0,16).c_str();
    //     fs.close();
    }
    //遍历变量
    for(set<string>::iterator it = varlist.begin(); it != varlist.end(); it ++) {
        string varaddr = cmdopen("cd " + kernelcodepath + "&&" + "grep -w " + *it + " System.map ");
        cmdopen("cd " + kernelcodepath + "&& echo " + *it + " >> result.txt");
        cmdopen("cd " + kernelcodepath + "&& echo " + varaddr.substr(0, 16) + " >> result.txt");
    }
    //最后删除中间文件
    //cmdopen("cd " + kernelcodepath + "&& " + "rm -rf asm.txt");
    return 0;
}
#include<iostream>
#include<vector>
using namespace std;
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
                cout<<"!!";
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
int main() {
    //识别变量
    //无等号
    // string theline = "static unsigned int challenge_count;";
    // string varname = theline.substr(theline.find_first_of(" ") + 1,theline.find_last_of(";") - theline.find_first_of(" ") - 1);
    // varname = varname.substr(varname.find_last_of(" "),varname.size() - varname.find_last_of(" "));
    // cout<<varname<<endl;
    //有等号
    // string theline = "-int sysctl_tcp_challenge_ack_limit = 100;";
    // string varname;
    // varname = theline.substr(theline.find_first_of(" ") + 1, theline.find_last_of("=") - theline.find_first_of(" ") -2);
    // if(varname.find(" ") != varname.npos) {
    //     varname = varname.substr(varname.find_last_of(" "),varname.size() - varname.find_last_of(" "));
    // }
    // cout<< varname<<endl;
    // vector<string>patchv;
    // patchv.push_back("@@ -87,7 +87,7 @@ int sysctl_tcp_adv_win_scale __read_mostly = 1;");
    // patchv.push_back("-int sysctl_tcp_challenge_ack_limit = 100;");
    // patchv.push_back("@@ -3458,7 +3458,7 @@ static void tcp_send_challenge_ack(struct sock *sk, const struct sk_buff *skb)");
    // string result = regvarlist(patchv, 0);
    // cout<<result;
    return 0;
}
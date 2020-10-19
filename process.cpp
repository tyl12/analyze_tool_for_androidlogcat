#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <functional>
#include <numeric>
#include <tuple>
#include <regex>
#include<getopt.h>
extern int optind,opterr,optopt;
extern char *optargi;

using namespace std;
namespace fs = std::filesystem;

bool debug=false;

tuple<string, long> time_parser(const string& input){
    //10-15 11:40:06.025
    auto timestr=input.substr(0,18);
    int month=stoi(input.substr(0,2));
    int day=stoi(input.substr(3,2));
    int hour=stoi(input.substr(6,2));
    int min=stoi(input.substr(9,2));
    int sec=stoi(input.substr(12,2));
    int millisec=stoi(input.substr(15,3));

    //FIXME
    long timeval=day*24*60*60*1000 + hour*60*60*1000 + min*60*1000 + sec*1000 + millisec;
    return make_tuple(timestr, timeval);
}

vector<long> get_package_timediff (const vector<string>& package){
    vector<long> time_parsed;
    for(const auto& s:package){
        auto [timestring, timelong] = time_parser(s);
        time_parsed.push_back(timelong);
        if(debug){
            cout<<s<<endl;
            cout<<"==> timestring: "<<timestring<<"\t\t timelong: "<<timelong<<endl;
        }
    }
    adjacent_difference(time_parsed.begin(), time_parsed.end(), time_parsed.begin());
    vector<long> time_diff(time_parsed.begin()+1, time_parsed.end());
    if (debug){
        cout<<"time difference:"<<endl;
        for (auto s:time_diff){
            cout<<s<<endl;
        }
    }

    return time_diff;
}

vector<string> split_by_delims(const string& str, const string& delims){
    string patternstr=string("[^")+delims+"]*["+delims+"]";
    cout<<"search pattern="<<patternstr<<endl;

    regex pattern(patternstr);
    vector<string> output;
    smatch result;
    string s = str;
    int i = 0;
    while (regex_search(s, result, pattern)){
        if (result.ready()){
            cout<<"index="<<i<<", result=["<<result[0]<<"]"<<endl;
            string tmp = result[0];
            char delim = *tmp.rbegin(); //or: tmp.back();
            if (delims.find(delim) != string::npos){
                tmp.erase(tmp.end()-1);
            }
            output.emplace_back(tmp);
            s = result.suffix();
            i++;
        }
    }
    if (s!="")
        output.emplace_back(s);
    return output;
}



int main(int argc, char* argv[]){
    int idx = 0;
    int c = 0; //用于接收选项
    /*循环处理参数*/
    // "-f logfile -s "adfasdfa@adfasd@sdfasdf" -i "a" -d
    static struct option long_options[] =
    {
        {"logfile",required_argument,NULL,'f'},
        {"stringtags",required_argument,NULL,'s'},
        {"seperator",required_argument,NULL,'i'},
        {"debugmode",no_argument,NULL,'d'}
    };


    bool strict_mode=false;
    string input_logfile="./logfile";
    string input_tagstrings="callback status : 0|稳定点达到，开始结算|上传中...|上传 成功";
    string seperator="|";

    while(EOF != (c = getopt_long(argc,argv,"f:s:i:d",long_options,&idx)))
    {
        switch(c)
        {
            case 'f':
                printf("we get option -f，%s\n",optarg);
                input_logfile=optarg;
                break;
            case 's':
                printf("we get option -s，%s\n",optarg);
                input_tagstrings=optarg;
                break;
            case 'i':
                printf("we get option -i, %s\n",optarg);
                seperator=optarg;
                break;
            case 'd':
                printf("we get option -d\n");
                debug = true;
                break;
            //表示选项不支持
            case '?':
                printf("unknow option:%c\n",optopt);
                break;
            default:
                break;
        }
    }

    vector<string> tags = split_by_delims(input_tagstrings, seperator);

    cout<<"process tags:"<<endl;
    for_each(tags.cbegin(), tags.cend(), [](const auto& s){cout<<"\t"<<s<<endl;});

    string intermediate_dir= "_intermediate";
    fs::path data_dir=fs::current_path()/=intermediate_dir;
    cout << "prepare data path: " << data_dir << endl;

    if ( fs::exists(data_dir))
        fs::remove_all(data_dir);
    fs::create_directories(data_dir);

    auto tags_cnt=tags.size();
    decltype(tags_cnt) index=0;

    //filter valid records, based on input tags, validate sequence as well
    vector<string> filter_strings;
    ifstream ifile(input_logfile);
    auto valid_line=[&](const string& line){
        for (const auto& tag:tags){
            if (line.find(tag)!=string::npos){
                return true;
            }
        }
        return false;
    };
    vector<string> package;
    vector<vector<string>> filter_packages;

    for (string line;getline(ifile, line);){
        if (!valid_line(line)){
            continue;
        }
        if (debug){
            cout<<"HANDLE tag index: " <<index<<endl;
        }
        if (line.find(tags[index]) != string::npos){//cache the matched sequence in package temporarily, to avoid any error happen in halfway
            package.emplace_back(line);
            if (index == tags.size() -1 ){
                if (debug){
                    cout<<"RECORD below valid package:"<<endl;
                    for_each(package.cbegin(), package.cend(), [ data_dir=data_dir](const string& s){cout<<s<<endl; });
                }
                filter_packages.emplace_back(package);
                filter_strings.insert(filter_strings.end(), make_move_iterator(package.begin()), make_move_iterator(package.end()));
                package.clear();
            }
            index = (index+1)%tags.size();
        }
        else{//something wrong
            if (filter_strings.size() == 0)
                continue; //wait for first match

            if (strict_mode){
                cout<<"ERROR to process file, some contents in wrong sequence found."<<endl;
                cout<<line<<endl;
                return -1;
            }
            else{
                if (debug){
                    cout<<"DROP below invalid package:"<<endl;
                    for_each(package.cbegin(), package.cend(), [](const string& s){cout<<s<<endl;});
                }
                package.clear();
                index = 0;
            }
        }
    }
    if (debug){
        auto filter_file = ofstream(data_dir/"_step1_filter_file");
        for_each(filter_strings.begin(), filter_strings.end(), [&](const string& s){filter_file<<s<<endl;});
    }

    //step 2: calculte the timestamp for each package
    vector<vector<long>> timediff_packages;
    for (const auto& package:filter_packages){
        auto timediff = get_package_timediff(package);
        timediff_packages.emplace_back(move(timediff));
    }
    if (debug){
        auto timediff_file = ofstream(data_dir/"_step2_timediff_file");
        for_each(timediff_packages.begin(), timediff_packages.end(),
                 [&](const auto& s){
                     for (const auto& i:s){
                         timediff_file<<i<<"\t";
                     }
                     timediff_file<<endl;
                 });
    }

    //average
    vector<vector<long>> timediff_stage(tags.size()-1);
    for(const auto& package:timediff_packages){
        for(int i=0;i<package.size();i++)
            timediff_stage[i].push_back(package[i]);
    }
    vector<long> timediff_stage_average;
    for (const auto& s:timediff_stage){
        timediff_stage_average.push_back(accumulate(s.cbegin(),s.cend(),0.0)/s.size());
    }

    //step 3: output
    printf("\n");
    for_each(tags.begin(), tags.end(), [](const auto& v){ printf("%-40s|", v.c_str());});
    printf("\n");
    for(const auto& package:timediff_packages){
        //printf("%-40s", " ");
        for_each(package.cbegin(), package.cend(), [](const auto& v){ printf("%-40d ", v);});
        printf("\n");
    }

    //step 4: statistics
    printf("\n");
    printf("Min:\n");
    for (const auto& s:timediff_stage){
        printf("%-40d ", *min_element(s.cbegin(), s.cend()));
    }
    printf("\n\n");

    printf("Max:\n");
    for (const auto& s:timediff_stage){
        printf("%-40d ", *max_element(s.cbegin(), s.cend()));
    }
    printf("\n\n");

    printf("AVERAGE:\n");
    for_each(timediff_stage_average.cbegin(), timediff_stage_average.cend(), [](const auto& v){ printf("%-40d ", v);});
    printf("\n\n");

    cout<<"done"<<endl;
    return 0;
}



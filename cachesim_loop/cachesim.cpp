#include <bits/stdc++.h>
#include <unistd.h>
#define ll long long
using namespace std;

struct ROW
{
    ll tag;
    bool v, d;
};

vector<vector<ROW>> cache;
vector<int> capacity; // 每组已有数据个数
vector<vector<int>> lru;

void check();
bool is2k(int);
int log2(int);
void init();
void r_miss(ll);
void r_hit(ll);
void w_miss(ll);
void w_hit(ll);
void work();
void filein(int, char *[]);
void output();

int blocksize, cachesize, misscost, relation_type, replace_type, alloc_type;
int groupnum, rownum;
int p1, p2, p3; // p1-tag; p2-group; p3-offset
ll runtime = 0, l_hit = 0, s_hit = 0, l_num=0, s_num=0;

string readfile1, readfile2, writefile;

int main(int argc, char *argv[])
{
    // filein(argc,argv);
    readfile1 = "cfg.txt";
    readfile2 = "ls.trace";
    writefile = "ls.trace.out";
    check();
    freopen("res","w",stdout);
    for(int i=0;i<6;i++){
        // blocksize=4<<i;
        relation_type=1<<i;
        // cachesize*=2;
        // replace_type=i;
        // alloc_type=i;
        cache.clear();capacity.clear();lru.clear();
        runtime = 0, l_hit = 0, s_hit = 0, l_num=0, s_num=0;
        init();
        work();
        // cout<<blocksize<<endl;
        output();

    }

    return 0;
}

bool is2k(int n)
{
    if (n <= 0)
        return false;
    while (n != 1)
    {
        if (n & 1)
            return false;
        n = n >> 1;
    }
    return true;
}
int log2(int n)
{
    int i = 0;
    while (n != 1)
    {
        i++;
        n = n >> 1;
    }
    return i;
}

void filein(int argc, char *argv[])
{
    int o;
    while ((o = getopt(argc, argv, "c:t:o:")) != -1)
    {
        switch (o)
        {
        case 'c':
            readfile1 = optarg;
            break;
        case 't':
            readfile2 = optarg;
            break;
        case 'o':
            writefile = optarg;
            break;
        }
    }
    if (writefile == "")
        writefile = readfile2 + ".out";
}

void check()
{
    ifstream fin1(readfile1);
    fin1 >> blocksize >> relation_type >> cachesize >> replace_type >> misscost >> alloc_type;
    if (!is2k(blocksize))
    {
        cout << "unright blocksize";
        exit(0);
    }
    if (!is2k(relation_type) && relation_type)
    {
        cout << "unright relation_type";
        exit(0);
    }
    if (!is2k(cachesize))
    {
        cout << "unright memry_size";
        exit(0);
    }
    if (replace_type != 0 && replace_type != 1)
    {
        cout << "unright replace type";
        exit(0);
    }
    if (misscost <= 0)
    {
        cout << "unright miss cost";
        exit(0);
    }
    if (alloc_type != 0 && replace_type != 1)
    {
        cout << "unright miss cost";
        exit(0);
    }
    fin1.close();
}

void init()
{
    if (relation_type == 0)
    {
        groupnum = 1;
        rownum = (1024 * cachesize) / blocksize;
    }
    else if (relation_type == 1)
    {
        groupnum = (1024 * cachesize) / blocksize;
        rownum = 1;
    }
    else
    {
        rownum = relation_type;
        groupnum = (1024 * cachesize) / blocksize / rownum;
        /*if(rownum==0){
            cout<<"relation type is too large";
            exit(0);
        }*/
        if (groupnum == 0)
        {
            groupnum = 1;
            rownum = (1024 * cachesize) / blocksize;
        }
    }
    ROW r;
    r.d = 0, r.tag = 0, r.v = 0;
    vector<ROW> vr;
    vector<int> vi;
    for (int i = 0; i < rownum; i++)
    {
        vr.push_back(r);
    }
    for (int i = 0; i < groupnum; i++)
    {
        cache.push_back(vr);
        capacity.push_back(0);
        lru.push_back(vi);
    }
    p3 = log2(blocksize);
    p2 = log2(groupnum);
    p1 = 48 - p2 - p3;
}

void r_miss(ll add)
{
    ll group = add >> p3;
    group = group & ((1 << p2) - 1);
    if (capacity[group] < cache[group].size())
    { // 不换
        int i = 0;
        for (; i < cache[group].size() && cache[group][i].v == 1; i++)
            ;
        cache[group][i].v = 1;
        cache[group][i].d = 0;
        cache[group][i].tag = (add >> (p3 + p2));
        capacity[group]++;
        runtime += misscost;
        lru[group].push_back(0);
        for (int ii = lru[group].size() - 1; ii; ii--)
        {
            lru[group][ii] = lru[group][ii - 1];
        }
        lru[group][0] = i;
    }
    else
    { // 换
        int dirty;
        if (replace_type == 0)
        {
            srand(time(0));
            int temp = rand() % capacity[group];
            dirty = cache[group][temp].d;
            cache[group][temp].d = 0;
            cache[group][temp].tag = add >> (p3 + p2);
            cache[group][temp].v = 1;
            runtime += misscost;
        }
        else
        {
            int temp = lru[group].back();
            for (int ii = lru[group].size() - 1; ii; ii--)
            {
                lru[group][ii] = lru[group][ii - 1];
            }
            lru[group][0] = temp;
            dirty = cache[group][temp].d;
            cache[group][temp].d = 0;
            cache[group][temp].tag = add >> (p3 + p2);
            cache[group][temp].v = 1;
            runtime += misscost;
        }
        // if(alloc_type==0 && dirty){//处理dirty位
        //     runtime+=misscost;
        // }
    }
}

void r_hit(ll add)
{
    runtime++;
    l_hit++;
    if (replace_type == 1)
    {
        ll group = add >> p3;
        group = group & ((1 << p2) - 1);
        ll tag = add >> (p3 + p2);
        for (int i = 0; i < rownum; i++)
        {
            if (tag == cache[group][i].tag && cache[group][i].v == 1)
            {
                int p;
                for (p = 0; p < rownum && lru[group][p] != i; p++)
                    ; // lru[group][p]==i
                for (int j = p; j; j--)
                {
                    lru[group][j] = lru[group][j - 1];
                }
                lru[group][0] = i;
                break;
            }
        }
    }
}

void w_miss(ll add)
{
    runtime += misscost;
    if (alloc_type == 0)
    {
        ll group = add >> p3;
        group = group & ((1 << p2) - 1);
        if (capacity[group] < cache[group].size())
        { // 不换
            int i = 0;
            for (; i < cache[group].size() && cache[group][i].v == 1; i++)
                ;
            cache[group][i].v = 1;
            cache[group][i].d = 1;
            cache[group][i].tag = (add >> (p3 + p2));
            capacity[group]++;
            lru[group].push_back(0);
            for (int ii = lru[group].size() - 1; ii; ii--)
            {
                lru[group][ii] = lru[group][ii - 1];
            }
            lru[group][0] = i;
        }
        else
        { // 换
            int dirty;
            if (replace_type == 0)
            {
                srand(time(0));
                int temp = rand() % capacity[group];
                dirty = cache[group][temp].d;
                cache[group][temp].d = 1;
                cache[group][temp].tag = add >> (p3 + p2);
                cache[group][temp].v = 1;
            }
            else
            {
                int temp = lru[group].back();
                for (int ii = lru[group].size() - 1; ii; ii--)
                {
                    lru[group][ii] = lru[group][ii - 1];
                }
                lru[group][0] = temp;
                dirty = cache[group][temp].d;
                cache[group][temp].d = 1;
                cache[group][temp].tag = add >> (p3 + p2);
                cache[group][temp].v = 1;
            }
            // if(alloc_type==0 && dirty){//处理dirty位
            //     runtime+=misscost;
            // }
        }
    }
}

void w_hit(ll add)
{
    s_hit++;

    if (alloc_type == 0)
        runtime++;
    else
        runtime +=misscost;

    ll group = add >> p3;
    group = group & ((1 << p2) - 1);
    ll tag = add >> (p3 + p2);
    for (int i = 0; i < rownum; i++)
    {
        if (tag == cache[group][i].tag && cache[group][i].v == 1)
        {
            cache[group][i].d = 1;
            if (replace_type == 1)
            {
                int p;
                for (p = 0; p < rownum && lru[group][p] != i; p++)
                    ; // lru[group][p]==i
                for (int j = p; j; j--)
                {
                    lru[group][j] = lru[group][j - 1];
                }
                lru[group][0] = i;
            }
            break;
        }
    }
}

void work()
{
    ifstream fin2(readfile2);
    string temps, t;
    char rw;
    stringstream sin;
    ll add, tag, group;
    bool flag = false;
    // fin2>>temps;
    while (fin2 >> t >> rw >> temps)
    {
        flag = false;
        sin.clear();
        sin << hex << temps;
        sin >> add;        
        if (rw == 'R')
        {
            l_num++;

            tag = add >> (p3 + p2);
            group = add >> p3;
            group = group & ((1 << p2) - 1);
            for (int i = 0; i < capacity[group]; i++)
            {
                if (cache[group][i].v == 1 && cache[group][i].tag == tag)
                {
                    flag = true;
                    break;
                }
            }
            if (flag)
                r_hit(add);
            else
                r_miss(add);
        }
        else
        {
            s_num++;

            tag = add >> (p3 + p2);
            group = add >> p3;
            group = group & ((1 << p2) - 1);
            for (int i = 0; i < capacity[group]; i++)
            {
                if (cache[group][i].v == 1 && cache[group][i].tag == tag)
                {
                    flag = true;
                    break;
                }
            }
            if (flag)
                w_hit(add);
            else
                w_miss(add);
        }
    }
    fin2.close();
}


void output()
{
    ofstream fout(writefile);
    cout <<  fixed << setprecision(4) << 100.0 * (l_hit + s_hit) / (l_num + s_num) << '%' << endl;
    // cout << "Load Hit Rate: " << fixed << setprecision(2) << 100.0 * l_hit / l_num << '%' << endl;
    // cout << "Store Hit Rate: " << fixed << setprecision(2) << 100.0 * s_hit / s_num << '%' << endl;
    // cout << "Total Run Time: " << runtime << endl;
    // cout << fixed << setprecision(2) << 1.0 * runtime / (l_num + s_num)<<endl;
    // cout<<'\n';
    // cout.close();
}
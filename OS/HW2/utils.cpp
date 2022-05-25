#include "utils.h"

vector<int> parser(){

    string s;
    getline(cin, s);


    if(s == ""){
        vector<int> res = {0};
        return res;
    }


    stringstream ss(s);

    int number;
    vector<int> res;

    while(ss >> number){
        res.push_back(number);
    }

    return res;
}

void order_parser(vector<pair<int,string>> & orders){

    string s;
    getline(cin, s);

    stringstream ss(s);

    string order;
    int time;

    ss >> time;
    ss >> order;

    orders.push_back({time,order});
}

proper_private::proper_private()
{
    vector<int> temp = parser();

    this->gid = temp[0];
    this->gather_area = {temp[1],temp[2]};
    this->delay_time = temp[3];
    this->area_count = temp[4];

    for (int i = 0; i < area_count; i++)
    {
        vector<int> temp1 = parser();
        pair<int,int> temp2 = {temp1[0],temp1[1]};

        this->areas.push_back(temp2);

        vector<int> temp3;
        this->mutex_map.push_back(temp3);
        this->smoker_map.push_back(temp3);
    }
}

proper_private::~proper_private()
= default;

int proper_private::getGid() const {
    return this->gid;
}

int proper_private::getDelay() const {
    return this->delay_time;
}

int proper_private::getAreaCount() const {
    return this->area_count;
}

pair<int, int> proper_private::getGatherArea() {
    return this->gather_area;
}

vector<pair<int, int>> proper_private::getAreas() {
    return this->areas;
}

vector<vector<int>> proper_private::getMutexMap() {
    return this->mutex_map;
}

void proper_private::updateMutexMap(int k, int index) {

    this->mutex_map[k].push_back(index);
}

void proper_private::updateSmokerMap(int k, int index) {
    this->smoker_map[k].push_back(index);
}

vector<vector<int>> proper_private::getSmokeMap() {
    return this->smoker_map;
}

bool find_overlap(pair<int,int> p1_tl, pair<int,int> p1_a, pair<int,int> p2_tl, pair<int,int> p2_a){

    if (p1_tl.first == p2_tl.first){

        if(p1_tl.second < p2_tl.second && (p1_tl.second + p1_a.second - 1) < p2_tl.second) return false;

        if(p2_tl.second < p1_tl.second && (p2_tl.second + p2_a.second -1) < p1_tl.second) return false;

        return true;
    }

    if(p1_tl.second == p2_tl.second){

        if(p1_tl.first < p2_tl.first && (p1_tl.first + p1_a.first - 1) < p2_tl.first) return false;

        if(p2_tl.first < p1_tl.first && (p2_tl.first + p2_a.first -1) < p1_tl.first) return false;

        return true;
    }

    if(p1_tl.first < p2_tl.first){

        if(p1_tl.second < p2_tl.second){

            return ((p1_tl.first + p1_a.first - 1) >= (p2_tl.first)) && ((p1_tl.second + p1_a.second - 1) >= (p2_tl.second));
        }

        return ((p1_tl.first + p1_a.first - 1) >= (p2_tl.first)) && ((p2_tl.second + p2_a.second - 1) >= (p1_tl.second));
    }

    if(p1_tl.first > p2_tl.first){

        if(p1_tl.second < p2_tl.second){

            return ((p2_tl.first + p2_a.first - 1) >= (p1_tl.first)) && ((p1_tl.second + p1_a.second - 1) >= (p2_tl.second));
        }

        return ((p2_tl.first + p2_a.first - 1) >= (p1_tl.first)) && ((p2_tl.second + p2_a.second - 1) >= (p1_tl.second));
    }
}

sneaky_smoker::sneaky_smoker() {
    vector<int> temp = parser();
    this->sid = temp[0];
    this->smoke_time = temp[1];
    this->cell_count = temp[2];

    for (int i = 0; i < this->cell_count; ++i) {
        vector<int> temp1 = parser();
        this->cells.push_back({temp1[0],temp1[1]});
        this->smoke_count.push_back(temp1[2]);

        vector<int> temp2;
        this->mutex_map.push_back(temp2);
    }
}

sneaky_smoker::~sneaky_smoker() = default;

int sneaky_smoker::getSid() const {
    return this->sid;
}

int sneaky_smoker::getSmokeTime() const {
    return this->smoke_time;
}

int sneaky_smoker::getCellCount() const {
    return this->cell_count;
}

vector<pair<int, int>> sneaky_smoker::getCells() {
    return this->cells;
}

int sneaky_smoker::getSmokeCount(int index) {
    return this->smoke_count[index];
}

void sneaky_smoker::updateMutexMap(int k, int index) {
    this->mutex_map[k].push_back(index);
}

vector<vector<int>> sneaky_smoker::getMutexMap() {
    return this->mutex_map;
}


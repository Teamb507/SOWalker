#ifndef _GRAPH_SORT_H_
#define _GRAPH_SORT_H_

#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include "api/graph_buffer.hpp"
#include "api/constants.hpp"
#include "api/types.hpp"
#include "logger/logger.hpp"
#include "util/util.hpp"
#include "util/io.hpp"
#include "precompute.hpp"
#include "split.hpp"
#include "engine/config.hpp"
#include <limits.h>
#include <unordered_map>
#include "metrics/metrics.hpp"
#include <queue>
#include <unordered_map>
#include "util/timer.hpp"
#include <random>
#include <algorithm>

struct pre_degree {
    vid_t vert;
    real_t degree_ratio;
    pre_degree():vert(0),degree_ratio(0){}
    pre_degree(vid_t a,real_t b):vert(a),degree_ratio(b){}
    friend bool operator < (const pre_degree &a, const pre_degree &b){
        if(a.degree_ratio!=b.degree_ratio){
            return a.degree_ratio<b.degree_ratio;
        }
        return a.vert>b.vert;
    }
};
bool cmp(const pre_degree &a,const pre_degree &b){
    if(a.degree_ratio==b.degree_ratio)
        return a.vert<b.vert;
    return a.degree_ratio>b.degree_ratio;
}

class graph_sort {
protected:
    std::string basename;
    std::string beg_pos_name;
    std::string csr_name;
    std::string degree_filename;
    vid_t nvertics;
    eid_t nedges;
    vid_t min_vert;
    vid_t verts_remain;
    pre_degree *deg;
    eid_t *vertbuffer;
    vid_t *edgebuffer;
    bool *pp;
    graph_buffer<vid_t> csr;
    graph_buffer<eid_t> beg_pos;
    graph_timer gtimer;
   
public:
    graph_sort(){}
    virtual void sort_graph() {}
    virtual void map_verts(){}
    ~graph_sort() {}
    void flush_file(std::string file_name){
        if(!beg_pos.empty()) {
            std::string name = get_beg_pos_name(file_name);
            appendfile(name, beg_pos.buffer_begin(), beg_pos.size());
            beg_pos.clear();
        }
        if(!csr.empty()){
            std::string name = get_csr_name(file_name);
            appendfile(name, csr.buffer_begin(), csr.size());
            csr.clear();
        }
    }
    void load_graph(){
        int vertdesc = open(beg_pos_name.c_str(), O_RDONLY);
        vertbuffer=(eid_t *)realloc(vertbuffer,(nvertics+1)*sizeof(eid_t));
        logstream(LOG_INFO) << "start load graph vert " <<std::endl;
        load_block_range(vertdesc, vertbuffer, (nvertics+1), 0*sizeof(eid_t));
        logstream(LOG_INFO) << "finish load graph vert " <<std::endl;
        int edgedesc = open(csr_name.c_str(), O_RDONLY);
        edgebuffer=(vid_t *)realloc(edgebuffer,nedges*sizeof(vid_t));
        logstream(LOG_INFO) << "start load graph edge " <<std::endl;
        load_block_range(edgedesc, edgebuffer, nedges, 0*sizeof(vid_t));
        logstream(LOG_INFO) << "finish load graph edge" <<std::endl;
    }
};
class BFS_sort:public graph_sort{
private:
    std::unordered_map<vid_t,real_t> verttodegree;
    std::unordered_map<vid_t,vid_t>  verttovert;
    std::unordered_map<vid_t,vid_t>  verttovert1;
    std::priority_queue<pre_degree> q; 

public:
    BFS_sort(graph_config *conf) :graph_sort(){
        basename = conf->base_name;
        nvertics = conf->nvertices;
        nedges = conf->nedges;
        min_vert = conf->min_vert;
        deg=NULL;
        vertbuffer=NULL;
        edgebuffer=NULL;
        pp=NULL;
        beg_pos.alloc(VERT_SIZE);
        csr.alloc(EDGE_SIZE);
        verts_remain=0;
        beg_pos_name = get_beg_pos_name(basename);
        csr_name = get_csr_name(basename);
        degree_filename=get_deg_name(basename);
    }
    ~ BFS_sort() {
        free(vertbuffer);
        free(edgebuffer);
        free(pp);
        free(deg);
        beg_pos.destroy();
        csr.destroy();
    }

    void map_verts(){
        int degdesc = open(degree_filename.c_str(), O_RDONLY);
        vid_t vert_map = lseek(degdesc, 0, SEEK_END) / sizeof(pre_degree);  //vert map表中元素个数
        logstream(LOG_INFO) << "vert_map size= " << vert_map<< std::endl;
        deg=(pre_degree *)realloc(deg, vert_map*sizeof(pre_degree));
        load_block_range(degdesc, deg, vert_map, 0*sizeof(pre_degree));
        for(auto i=0;i<vert_map;i++){
            verttodegree.emplace(deg[i].vert,deg[i].degree_ratio);
        }
        vid_t totalvert=nvertics;
        logstream(LOG_INFO) << "totalvert= " << totalvert<< std::endl;
        pp=(bool *)realloc(pp,totalvert*sizeof(bool));
        memset(pp,false,totalvert);
        bool *&p=pp;
        q.push(deg[0]);
        assert((deg[0].vert-min_vert)<totalvert);
        p[deg[0].vert-min_vert]=true;
        vid_t current_vert,offindex,tmp,vert_mapped;
        eid_t left,right;
        current_vert=offindex=tmp=vert_mapped=0;
        left=right=0;
        load_graph();
        gtimer.start_time();
        while(!q.empty()){                  
            current_vert=q.top().vert;
            /*if(vert_mapped%100000==0){
                logstream(LOG_INFO) << "vert_map count = " <<vert_mapped<< std::endl;
            }*/
            verttovert.emplace(current_vert,vert_mapped);
            verttovert1.emplace(vert_mapped,current_vert);
            vert_mapped++;
            q.pop();
            //offindex=current_vert-min_vert;     
            offindex=current_vert;           
            left=vertbuffer[offindex];
            right=vertbuffer[offindex+1];
            for(auto i=left;i<right;i++){
                if(p[edgebuffer[i]-min_vert]==0){
                    q.push(pre_degree(edgebuffer[i],verttodegree[edgebuffer[i]]));
                    p[edgebuffer[i]-min_vert]=1;
                }
            }
            if(q.empty()||left==right){
                while(p[deg[tmp].vert-min_vert]&&tmp<vert_map-1){
                    tmp++;
                }
                if(p[deg[tmp].vert-min_vert]==0){
                    q.push(deg[tmp]);
                    p[deg[tmp].vert-min_vert]=1;
                }
            }      
        }
        for(auto i=0;i<totalvert;i++){
            if(p[i]==0){
                verts_remain++;
            }
        }
        logstream(LOG_DEBUG) << "map time : " << gtimer.runtime() << std::endl;
        gtimer.stop_time();
        logstream(LOG_INFO) << "vert has been mapped is " << verttovert.size() <<";verts has not been mapped "<<verts_remain<<std::endl;
    }
    void sort_graph(){
        map_verts();
        std::string new_dir=basename.substr(0,basename.find_last_of("/"))+"/BFS/";
        system (("mkdir "+new_dir).c_str());
        std::string new_basename=new_dir+"graph";
        logstream(LOG_INFO) << new_basename<<std::endl;
        eid_t left,right,adj_num,csr_pos;
        vid_t current_vert,offindex;
        left=right=adj_num=csr_pos=0;
        current_vert=offindex=0;
        beg_pos.push_back(csr_pos);
        for(auto i=0;i<verttovert1.size();i++){
            current_vert=verttovert1[i];
            offindex=current_vert;
            left=vertbuffer[offindex];
            right=vertbuffer[offindex+1];
            csr_pos+=(right-left);
            beg_pos.push_back(csr_pos);
            adj_num=right-left;
            if(beg_pos.full()||csr.test_overflow(adj_num)||csr.full()){
                flush_file(new_basename);
            }
            for(auto j=left;j<right;j++){
                csr.push_back(verttovert[edgebuffer[j]]);
            }
        }
        flush_file(new_basename); 
        for(auto i=1;i<=verts_remain;i++){
            beg_pos.push_back(csr_pos);
            if(beg_pos.full()){
                flush_file(new_basename);
            }
        }
        flush_file(new_basename); 
        system(("cp "+basename+".meta "+new_dir).c_str());
        system(("mv "+new_dir+basename.substr(basename.find_last_of("/"))+".meta "+new_dir+"graph.meta").c_str());
    }
};
class Bounded_BFS_sort:public graph_sort{
private:
    std::unordered_map<vid_t,real_t> verttodegree;
    std::unordered_map<vid_t,vid_t>  verttovert;
    std::unordered_map<vid_t,vid_t>  verttovert1;
    std::priority_queue<pre_degree> q; 
    vid_t bound;

public:
    Bounded_BFS_sort(graph_config *conf,vid_t b) :graph_sort(){
        basename = conf->base_name;
        nvertics = conf->nvertices;
        nedges = conf->nedges;
        min_vert = conf->min_vert;
        deg=NULL;
        vertbuffer=NULL;
        edgebuffer=NULL;
        pp=NULL;
        beg_pos.alloc(VERT_SIZE);
        csr.alloc(EDGE_SIZE);
        verts_remain=0;
        beg_pos_name = get_beg_pos_name(basename);
        csr_name = get_csr_name(basename);
        degree_filename=get_deg_name(basename);
        bound=b;
    }
    ~ Bounded_BFS_sort() {
        free(vertbuffer);
        free(edgebuffer);
        free(pp);
        free(deg);
        beg_pos.destroy();
        csr.destroy();
    }
    void map_verts(){
        int degdesc = open(degree_filename.c_str(), O_RDONLY);
        vid_t vert_map = lseek(degdesc, 0, SEEK_END) / sizeof(pre_degree); 
        logstream(LOG_INFO) << "vert_map size= " << vert_map<< std::endl;
        deg=(pre_degree *)realloc(deg, vert_map*sizeof(pre_degree));
        load_block_range(degdesc, deg, vert_map, 0*sizeof(pre_degree));
        for(auto i=0;i<vert_map;i++){
            verttodegree.emplace(deg[i].vert,deg[i].degree_ratio);
        }
        vid_t totalvert=nvertics;
        logstream(LOG_INFO) << "totalvert= " << totalvert<< std::endl;

        pp=(bool *)realloc(pp,totalvert*sizeof(bool));
        memset(pp,false,totalvert);
        bool *&p=pp;

        vid_t current_vert,offindex,tmp,vert_mapped,offindex1;
        eid_t left,right,left1,right1;
        current_vert=offindex=tmp=vert_mapped=offindex1=0;
        left=right=left1=right1=0;
        load_graph();
        gtimer.start_time();
        vid_t temp_curvert=0;
        for(auto i=0;i<vert_map;i++){   
            if(p[deg[i].vert-min_vert]==0){    
                q.push(deg[i]);
                assert((deg[i].vert-min_vert)<totalvert);
                p[deg[i].vert-min_vert]=true;
                //offindex=deg[i].vert-min_vert;        
                offindex=deg[i].vert;      
                left=vertbuffer[offindex];
                right=vertbuffer[offindex+1];
                for(auto j=left;j<right;j++){
                    temp_curvert=edgebuffer[j];
                    //offindex1=temp_curvert-min_vert;    
                    offindex1=temp_curvert;                  
                    left1=vertbuffer[offindex1];
                    right1=vertbuffer[offindex1+1];
                    for(auto k=left1;k<right1;k++){ 
                        if(p[edgebuffer[k]-min_vert]==0){
                            q.push(pre_degree(edgebuffer[k],verttodegree[edgebuffer[k]]));
                            p[edgebuffer[k]-min_vert]=1;
                        }
                    }
                }
                while(!q.empty()){                  
                    current_vert=q.top().vert;
                    if(vert_mapped%100000==0){
                        logstream(LOG_INFO) << "vert_map count = " <<vert_mapped<< std::endl;
                    }
                    verttovert.emplace(current_vert,vert_mapped);
                    verttovert1.emplace(vert_mapped,current_vert);
                    vert_mapped++;
                    q.pop();
                }
            }
        }

        for(auto i=0;i<totalvert;i++){
            if(p[i]==0){
                verts_remain++;
            }
        }
        logstream(LOG_DEBUG) << "map time : " << gtimer.runtime() << std::endl;
        gtimer.stop_time();
        logstream(LOG_INFO) << "vert has been mapped is " << verttovert.size() <<";verts has not been mapped "<<verts_remain<<std::endl;
    }

    void sort_graph(){
        map_verts();
        std::string new_dir=basename.substr(0,basename.find_last_of("/"))+"/BBFS/";
        system (("mkdir "+new_dir).c_str());
        std::string new_basename=new_dir+"graph";
        logstream(LOG_INFO) << new_basename<<std::endl;
        eid_t left,right,adj_num,csr_pos,offindex;
        vid_t current_vert;
        left=right=adj_num=csr_pos=0;
        current_vert=offindex=0;
        beg_pos.push_back(csr_pos);
        for(auto i=0;i<verttovert1.size();i++){
            current_vert=verttovert1[i];  
            offindex=current_vert;
            left=vertbuffer[offindex];
            right=vertbuffer[offindex+1];
            csr_pos+=(right-left);
            beg_pos.push_back(csr_pos);
            adj_num=right-left;
            if(beg_pos.full()||csr.test_overflow(adj_num)||csr.full()){
                flush_file(new_basename);
            }
            for(auto j=left;j<right;j++){
                csr.push_back(verttovert[edgebuffer[j]]);
            }
        }
        flush_file(new_basename); 
        for(auto i=1;i<=verts_remain;i++){
            beg_pos.push_back(csr_pos);
            if(beg_pos.full()){
                flush_file(new_basename);
            }
        }
        flush_file(new_basename); 
        system(("cp "+basename+".meta "+new_dir).c_str());
        system(("mv "+new_dir+basename.substr(basename.find_last_of("/"))+".meta "+new_dir+"graph.meta").c_str());
    }
};

class Random_sort:public graph_sort{
private:
    std::unordered_map<vid_t,real_t> verttodegree;
    std::unordered_map<vid_t,vid_t>  verttovert;
    std::unordered_map<vid_t,vid_t>  verttovert1;
    std::priority_queue<pre_degree> q; 
    std::vector<pre_degree> deg_vector;
public:
    Random_sort(graph_config *conf) :graph_sort(){
        basename = conf->base_name;
        nvertics = conf->nvertices;
        nedges = conf->nedges;
        min_vert = conf->min_vert;
        deg=NULL;
        vertbuffer=NULL;
        edgebuffer=NULL;
        pp=NULL;
        beg_pos.alloc(VERT_SIZE);
        csr.alloc(EDGE_SIZE);
        verts_remain=0;
        beg_pos_name = get_beg_pos_name(basename);
        csr_name = get_csr_name(basename);
        degree_filename=get_deg_name(basename);
    }
    ~ Random_sort() {
        free(vertbuffer);
        free(edgebuffer);
        free(pp);
        free(deg);
        beg_pos.destroy();
        csr.destroy();
    }

    void map_verts(){
        int degdesc = open(degree_filename.c_str(), O_RDONLY);
        vid_t vert_map = lseek(degdesc, 0, SEEK_END) / sizeof(pre_degree);  
        logstream(LOG_INFO) << "vert_map size= " << vert_map<< std::endl;
        vid_t totalvert=nvertics;
        logstream(LOG_INFO) << "totalvert= " << totalvert<< std::endl;
        deg=(pre_degree *)realloc(deg, vert_map*sizeof(pre_degree));
        load_block_range(degdesc, deg, vert_map, 0*sizeof(pre_degree));
        for(auto i=0;i<vert_map;i++){
            deg_vector.push_back(deg[i]);
        }
        vid_t current_vert,offindex,tmp,vert_mapped;
        eid_t left,right;
        current_vert=offindex=tmp=vert_mapped=0;
        left=right=0;
        auto rand_pos = 0;
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(deg_vector.begin(),deg_vector.end(), rng);
        pp=(bool *)realloc(pp,totalvert*sizeof(bool));
        memset(pp,false,totalvert);
        bool *&p=pp;
        load_graph();
        RandNum *seed=new RandNum(1);
        gtimer.start_time(); 
        for(auto i=0;i<deg_vector.size();i++){
            current_vert=deg_vector[i].vert;
            p[current_vert-min_vert]=true;
            verttovert.emplace(current_vert,vert_mapped);
            verttovert1.emplace(vert_mapped,current_vert);
            vert_mapped++;
            if(vert_mapped%100000==0){
                logstream(LOG_INFO) << "vert_map count = " <<vert_mapped<< std::endl;
            }
        }
        for(auto i=0;i<totalvert;i++){
            if(p[i]==0){
                verts_remain++;
            }
        }
        logstream(LOG_DEBUG) << "map time : " << gtimer.runtime() << std::endl;
        gtimer.stop_time();
        logstream(LOG_INFO) << "vert has been mapped is " << verttovert.size() <<";verts has not been mapped "<<verts_remain<<std::endl;
    }

    void sort_graph(){
        map_verts();
        std::string new_dir=basename.substr(0,basename.find_last_of("/"))+"/Random/";
        system (("mkdir "+new_dir).c_str());
        std::string new_basename=new_dir+"graph";
        logstream(LOG_INFO) << new_basename<<std::endl;
        eid_t left,right,adj_num,csr_pos,offindex;
        vid_t current_vert;
        left=right=adj_num=csr_pos=0;
        current_vert=offindex=0;
        beg_pos.push_back(csr_pos);
        for(auto i=0;i<verttovert1.size();i++){
            current_vert=verttovert1[i];   
            offindex=current_vert;
            left=vertbuffer[offindex];
            right=vertbuffer[offindex+1];
            csr_pos+=(right-left);
            beg_pos.push_back(csr_pos);
            adj_num=right-left;
            if(beg_pos.full()||csr.test_overflow(adj_num)||csr.full()){
                flush_file(new_basename);
            }
            for(auto j=left;j<right;j++){
                csr.push_back(verttovert[edgebuffer[j]]);
            }
        }
        flush_file(new_basename); 
        for(auto i=1;i<=verts_remain;i++){
            beg_pos.push_back(csr_pos);
            if(beg_pos.full()){
                flush_file(new_basename);
            }
        }
        flush_file(new_basename); 
        system(("cp "+basename+".meta "+new_dir).c_str());
        system(("mv "+new_dir+basename.substr(basename.find_last_of("/"))+".meta "+new_dir+"graph.meta").c_str());
    }
};
#endif
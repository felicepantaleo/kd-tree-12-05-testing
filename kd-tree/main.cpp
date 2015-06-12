//
//  main.cpp
//  kd-tree-12-05
//
//  Created by Ann-Christine Vossberg on 5/12/15.
//  Copyright (c) 2015 Ann-Christine Vossberg. All rights reserved.
//

#include <stdio.h>
#include <vector>
#include "KD_tree.hpp"
#include "KD_tree.cpp"
#include "simple_kd_tree.hpp"
#include "simple_kd_tree.cpp"
#include "cuda_runtime.h"
#include "cuda.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <future>
//#include "test.hpp"
#include "InsideBox.hpp"

#define MYDEVICE 0

using namespace std;


template <typename num_t>
void generateRandomPointCloud( vector<Point<num_t>> &point, const size_t N, const int max_range = 10)
{
    cout << "Generating "<< N << " point cloud...\n";
    point.resize(N);
    for (size_t i=0;i<N;i++)
    {
        point[i].x = max_range * (rand() % 1000) / num_t(1000);
        point[i].y = max_range * (rand() % 1000) / num_t(1000);
        point[i].z = max_range * (rand() % 1000) / num_t(1000);
        point[i].ID = i;
        //cout << point[i].x << ", " << point[i].y << ", " << point[i].z << " ID: " << point[i].ID << endl;
        
    }
    std::cout << "done\n \n";
}

template <typename num_t>
bool test(vector<Point<num_t>> treevector, SimpleKDtree<num_t>* Tsimple){
    //cout << "testing ..." << endl;
    
    if(Tsimple->sameTree(treevector, 1)){
        //cout << "yay! trees are the same" << endl;
        return true;
    }
    else{
        cout << "o oh - debug some more!" << endl;
        
        return false;
    }
}
template <typename num_t>
void print_Pointvector(vector<Point<num_t>> a){
    for(int i = 0; i<a.size(); i++){
        cout << "( "<< a[i].x << ", " << a[i].y << ", " << a[i].z << ")" << endl;
    }
    cout << "\n" << endl;
}

template <typename num_t>
void make_reference_tree(vector<Point<num_t>> cloud, vector<int> dimensions, vector<vector<Point<num_t>>> &trees, int Id){
    
}

template <typename num_t>
void make_tree(vector<Point<num_t>> cloud, vector<int> dimensions, vector<vector<Point<num_t>>> &trees, int Id){
    KD_tree<num_t> tree(cloud, dimensions);
    tree.KD_tree_recursive(0, cloud.size()-1, 0, 1);
    trees[Id] = tree.get_tree_as_vector();
}

template <typename num_t>
vector<vector<Point<num_t>>> make_forest(vector<Point<num_t>> &cloud,vector<int> dimensions, int datapoints_per_tree, int nthreads){
    vector<vector<Point<num_t>>> trees(nthreads);
    vector<std::future<void>> futures;
    
    for(int id = 0; id < nthreads; ++id){
        //TODO: auch aufsplitten - das kann jeder thread selbst tun
        //TODO: maybe way to use part of vector without copying
        
        vector<Point<num_t>> threadcloud(cloud.begin()+id*datapoints_per_tree, cloud.begin()+(id+1)*datapoints_per_tree);
        futures.push_back(std::async(launch::async, make_tree<num_t>, threadcloud, dimensions, std::ref(trees), id));
        
    }
    
    for(auto &e : futures) {
        e.get();
    }
    
    return trees;
}

template <typename num_t>
vector<int> inBox(Point<num_t> start, Point<num_t> end, vector<vector<Point<num_t>>> &trees ){
    vector<int> result;
    
    //achtung! letzter trees ist evtl. kleiner und nicht 32...
    for(int i=0; i<trees.size(); i++){
        for(int j = 0; j< trees[i].size();j++){
            
            if( ((trees[i][j].x >= start.x && trees[i][j].x <= end.x) || (end.x == 0 && start.x == 0))  && ((trees[i][j].y >= start.y && trees[i][j].y <= end.y) || (end.y == 0 && start.y == 0)) && ((trees[i][j].z >= start.z && trees[i][j].z <= end.z) || (end.z == 0 && start.z == 0))){
                result.push_back(trees[i][j].ID);
            }
        
        }
    
    }
    return result;
}

void printDevProp(cudaDeviceProp devProp)
{
    //http://www.cs.fsu.edu/~xyuan/cda5125/examples/lect24/devicequery.cu
    printf("Major revision number:         %d\n",  devProp.major);
    printf("Minor revision number:         %d\n",  devProp.minor);
    printf("Name:                          %s\n",  devProp.name);
    printf("Total global memory:           %u\n",  devProp.totalGlobalMem);
    printf("Total shared memory per block: %u\n",  devProp.sharedMemPerBlock);
    printf("Total registers per block:     %d\n",  devProp.regsPerBlock);
    printf("Warp size:                     %d\n",  devProp.warpSize);
    printf("Maximum memory pitch:          %u\n",  devProp.memPitch);
    printf("Maximum threads per block:     %d\n",  devProp.maxThreadsPerBlock);
    for (int i = 0; i < 3; ++i)
        printf("Maximum dimension %d of block:  %d\n", i, devProp.maxThreadsDim[i]);
    for (int i = 0; i < 3; ++i)
        printf("Maximum dimension %d of grid:   %d\n", i, devProp.maxGridSize[i]);
    printf("Clock rate:                    %d\n",  devProp.clockRate);
    printf("Total constant memory:         %u\n",  devProp.totalConstMem);
    printf("Texture alignment:             %u\n",  devProp.textureAlignment);
    printf("Concurrent copy and execution: %s\n",  (devProp.deviceOverlap ? "Yes" : "No"));
    printf("Number of multiprocessors:     %d\n",  devProp.multiProcessorCount);
    printf("Kernel execution timeout:      %s\n",  (devProp.kernelExecTimeoutEnabled ? "Yes" : "No"));
    
    return;
}




int main()
{
    //: <int> to generic --> DONE
    //: check, which tree should be built.. cut top or bottom? ->TOP ->DONE
    //: make test compare to other tree -> DONE
    //: schön alles in class machen --> DONE
    //: test for 3D -> DONE
    //: test for larger - random trees. -> DONE
    //: test for double/float -> DONE
    //TODO: free kd_tree.. ?
    
    //type to use:
    typedef int num_t;
    
    vector<Point<num_t>> cloud;
    
    // Generate points:
    int numberOfHits = 1000;
    generateRandomPointCloud(cloud, numberOfHits);
    
    //must be defined {1, 2, 3} = {x, y, z}
    vector<int> dimensions = {1,2,3};
    
    //get_size_of_tree from cuda_device --> #datapoints per thread.. = datapoints per tree
    int device;
    cudaGetDevice(&device);
    std::cout<< "devices are: " << device << endl;
    
    cudaDeviceProp devProp;
    cudaGetDeviceProperties(&devProp, device);
    printDevProp(devProp);
    int max_threads = devProp.warpSize;
    //TODO: here calculate #nodes need
    
    cout << "number of warps " << max_threads << endl;
    
    int warp_size = max_threads;
    
    //TODO: datapoints_per_tree must me calculated:
    //they must be less than max threads per block, because of extra empty nodes.
    //???: will it be one tree per warp or per block?
    //formula: 2^(floor(log_2(64)+1))-1 is always max when one less than warp_size
    int datapoints_per_tree = warp_size-1;
    
    int threads = cloud.size()/datapoints_per_tree;
    vector<vector<Point<num_t>>> trees = make_forest<num_t>(cloud, dimensions, datapoints_per_tree, threads);
    
    //print
    /*for(int i = 0; i< trees.size(); i++){
        print_Pointvector(trees[i]);
    }*/
    
    //TODO: hier fehler!!! nicht alle datapoints werden benützt - einige gehen verloren!
    bool correctTree=true;
    for(int i = 0; i < threads; i++){
        SimpleKDtree<num_t> *bst = new SimpleKDtree<num_t>(dimensions);
        vector<Point<num_t>> threadcloud (cloud.begin()+i*datapoints_per_tree, cloud.begin()+(i+1)*datapoints_per_tree);
        bst->make_SimpleKDtree(threadcloud, 0, threadcloud.size()-1, 0);
        correctTree = correctTree && test(trees[i], bst);
        delete bst;
    }
    if(correctTree){cout << "\nAll tree's are correct" << endl; }
    
    
    //make trees into array (instead vector<vector< >> and copy this array over
    //TODO: should be done while making trees and not converted afterwards
    
    int* treeArray_x = new int[trees.size()*trees[0].size()];
    int* treeArray_y = new int[trees.size()*trees[0].size()];
    int* treeArray_z = new int[trees.size()*trees[0].size()];
    int* treeArray_ID = new int[trees.size()*trees[0].size()];
    
    
    //cout << " number of trees " << trees.size() << endl;
    for(int i=0; i< trees.size() ; i++){
        //cout << "size of tree: at " << i << " is " << trees[i].size() << endl;
        for(int j = 0; j < trees[i].size(); j++){
            //TODO: stimmt das trees[i].size() ? müsste nicht trees[0]? trees[i].size() = #nodes dieses trees i. #nodes müssten für jeden tree gleich sein!!!
            treeArray_x[i*trees[i].size()+j] = trees[i][j].x;
            treeArray_y[i*trees[i].size()+j] = trees[i][j].y;
            treeArray_z[i*trees[i].size()+j] = trees[i][j].z;
            treeArray_ID[i*trees[i].size()+j] = trees[i][j].ID;
        }
    }
    int size_of_forest = sizeof(int)*trees.size()*trees[0].size();
    
    //check array: - wieder weg!
    for(int i = 0; i < 992; i++){
        cout << treeArray_x[i] << endl;
    }
    //make box, in which should be searched for hits
    //set all other dimensions to zero, if not used:
    Point<num_t> box_start;
    Point<num_t> box_end;
    box_start.x = box_start.y = box_start.z = box_end.x = box_end.y = box_end.z = 0;
    box_start.x = 0;
    box_end.x = 10;
    
    
    cout << box_start.x << " " << box_start.y << " " << box_start.z << endl;
    cout << box_end.x << " " << box_end.y << " " << box_end.z << endl;
    //x_start, x_end, y_start, y_end, z_start, z_end
    num_t box[6] = {2, 8, 0, 0, 0, 0};
    
    
    
    
    
    //-------start with CUDA----------------------------
    /*num_t *d_treeArray_x;
    num_t *d_treeArray_y;
    num_t *d_treeArray_z;
    int *d_treeArray_ID;
    int size_of_forest = sizeof(num_t)*trees.size()*trees[0].size();
    
    
    //allocate memory
    cudaMalloc(&d_treeArray_x, size_of_forest);
    cudaMalloc(&d_treeArray_y, size_of_forest);
    cudaMalloc(&d_treeArray_z, size_of_forest);
    cudaMalloc(&d_treeArray_ID, size_of_forest);

    //send trees to gpu
    cudaMemcpy(d_treeArray_x, treeArray_x, size_of_forest, cudaMemcpyHostToDevice);
    cudaMemcpy(d_treeArray_y, treeArray_y, size_of_forest, cudaMemcpyHostToDevice);
    cudaMemcpy(d_treeArray_z, treeArray_z, size_of_forest, cudaMemcpyHostToDevice);
    cudaMemcpy(d_treeArray_ID, treeArray_ID, size_of_forest, cudaMemcpyHostToDevice);
    //here we have to split the forest? - split on GPU?
    
    
    //allocate host and device memory for results - ID's of hits/datapoints inside box
    num_t* h_result;
    num_t* d_result;
    size_t resultSize = numberOfHits*sizeof(num_t);
    h_result = (num_t *) malloc(resultSize);
    cudaMalloc((void **)&d_result, resultSize);
    
    
    //TODO:kernel, s.d. jeder einzelne thread checkt, ob in box - box-dimensionen gegeben -
    //gibt zurück ein array mit punkten, die in box (coordinaten? ID's? .. )
    //main.cpp -> main.cu und andere compilation von c++11 zeug muss ausgelagert werden
    //search forest for points inside box_dimensions
    
    vector<int> result = inBox(box_start, box_end, trees);
    /*cout << "inBox are:\n " << endl;
    for(int i = 0; i< result.size(); i++){
        cout << result[i] << "\n" << endl;
    }*/
    
    //-------------------test mit vorhandenem .cu-file--------------------------
    
    //test();-- hat funktioniert
    cudaMain(trees.size(), trees[0].size(), treeArray_x, treeArray_y, treeArray_z, treeArray_ID, box);
    
    cloud.clear();
    
    return 0;
}
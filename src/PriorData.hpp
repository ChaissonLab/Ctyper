//
//  Created by Walfred (Wangfei) MA at the University of Southern California,
//  Mark Chaisson Lab on 2/13/23.
//
//  Licensed under the MIT License. 
//  If you use this code, please cite our work.
//   

#ifndef PriorData_hpp
#define PriorData_hpp

#include <stdio.h>
#include <vector>
#include <utility>
#include <unordered_set>

#include "config.hpp"
#include "KmerCounter.hpp"
#include "Regression.hpp"
#include "TreeRound.hpp"
#include "KtableReader.hpp"

//#define buffer_size 200

using namespace std;

template<typename T>
void try_allocate(T*& ptr, size_t size, size_t reserve = 0)
{
    std::unique_ptr<T> temp_ptr; //require free memory before proceed
    int attempts = 200;
    while (attempts-- > 0) 
    {
        try 
	{
            temp_ptr.reset(new T[reserve]);
        } 
	catch (const std::bad_alloc&) 
	{
            std::cerr << "Initial memory reservation failed. " << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    attempts = 200;
    T* new_ptr = nullptr;

    while (attempts-- > 0)
    {
        new_ptr = (T*)realloc(ptr, sizeof(T) * size);
        if (new_ptr != nullptr) {
            ptr = new_ptr;
            return;
        }

        std::cerr << "Allocation retrying, Attempt: " << 200 - attempts << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    std::cerr << "Failed to allocate memory after 200 attempts. Exiting." << std::endl;
    std::exit(EXIT_FAILURE);
}

template<typename T>
void try_allocate_unique(std::unique_ptr<T>& uptr, size_t size, size_t reserve = 0)
{
    int attemps = 200;
    
    while (attemps-- > 0)
    {
        try
        {
            std::unique_ptr<T> temp_ptr(new T[reserve]); //require twice memory before proceed
            uptr.reset(new T[size]);
            
            return;
        }
        catch (const std::bad_alloc&)
        {
            std::cerr << "Allocation retrying, Attemp: " << 200 - attemps << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    std::cerr << "Failed to allocate memory after 200 attempts. Exiting." << std::endl;
    std::exit(EXIT_FAILURE);
}

struct PriorChunk
{
    
    ~PriorChunk()
    {
        free(gene_kmercounts);
        free(kmer_matrix);
        free(prior_norm);
        free(phylo_tree);
    }
    
    size_t index = 0;
    
    string prefix = "";
    size_t genenum = 0;
    size_t numgroups = 0;
    
    vector<string> genenames;
    vector<uint16> genegroups;
    vector<uint> groupkmernums;
   
    vector<string> pathnames; 
    vector<uint> pathsizes;
    
    FLOAT_T* prior_norm= NULL;
    size_t prior_norm_allocsize = 0;
    
    uint16* kmer_matrix = NULL;
    size_t kmer_matrix_allocsize = 0;
    
    node* phylo_tree = NULL;
    size_t nodenum = 0;
    size_t phylo_tree_allocsize = 0;
    
    uint* gene_kmercounts = NULL;
    size_t gene_kmercounts_allocsize = 0;
    
    size_t kmervec_start= 0, kmervec_size = 0;

};



class PriorData
{
    
public:
    
    PriorData(const string &path, const size_t b = 200): datapath(path), file(path.c_str()), buffer_size(b)
    {};
    ~PriorData()
    {}
    
    size_t LoadIndex(const unordered_set<string>& geneset);
    size_t LoadIndex();
    
    void LoadFile();
    void CloseFile();

    PriorChunk* getChunkData(const size_t Chunkindex);
    PriorChunk* getNextChunk(const vector<bool>& finished);
    void FinishChunk(PriorChunk* Chunk_prt);

	size_t totalkmers = 0 ;

private:
    
    void LoadHeader(PriorChunk &Chunk);
    void LoadAlleles(PriorChunk &Chunk);
    void LoadSizes(PriorChunk &Chunk);
    void LoadMatrix(PriorChunk &Chunk, size_t index_size);
    void LoadNorm(PriorChunk &Chunk);
    void LoadGroups(PriorChunk &Chunk);
    void LoadTree(PriorChunk &Chunk);
    
    PriorChunk* getFreeBuffer(size_t Chunkindex);
    
    vector<string> prefixes;
    vector<pair<size_t,size_t>> kmervec_pos;
    vector<pair<size_t,size_t>> file_pos;
    vector<size_t> indexed_matrix_sizes;
    
    const size_t buffer_size; //const variable will be initialized firstly than non-const.
    vector<PriorChunk> Buffers = vector<PriorChunk>( buffer_size + 10);
    vector<size_t> Buffer_indexes = vector<size_t>( buffer_size + 10, INT_MAX);
    vector<size_t> Buffer_working_counts = vector<size_t>( buffer_size + 10, 0) ;
    
    size_t buff_index = 0, total_buff;
    const string datapath;
    
    size_t LoadRow(uint16* matrix, size_t index, string &StrLine, vector<uint> &pathsizes);
    KtableReader file;
    std::mutex IO_lock;
    
};


#endif /* PriorData_hpp */

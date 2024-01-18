#include "scheduler.h"
#include "common.h"
#include <iostream>
#include <algorithm>

// this is the function you should implement
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrival_times, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//



void simulate_rr(
    int64_t quantum, 
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) {
    seq.clear();

    int64_t curr_time = 0;
    int cpu = -1;
    std::vector<int> readyQueue, jobQueue; // index of procs
    std::vector<int64_t> remaining_bursts; // remaining bursts
    std::vector<bool>  started; // proc start
        
    int64_t remaining = quantum;
    int current = cpu; // set the current process to the cpu - idle

    // set all to the size of the processes
    started.resize((int)processes.size());
    remaining_bursts.resize((int)processes.size());
    jobQueue.resize((int)processes.size());

    for(int i=0; i < (int)processes.size(); i++){
        jobQueue[i] = processes[i].id;
        started[i] = false; // set all processes to not started
        remaining_bursts[i] = processes[i].burst; // set all remaining burst time to inital burst time
    }
    int jobs_remaining = jobQueue.size();

    while(1){

        // when all jobs done end loop
        if(jobs_remaining == 0){
            break;
        }
        
        // end and set current proc to idle
        if(current != cpu && remaining_bursts[current] == 0){
            processes[current].finish_time = curr_time;
            jobs_remaining--;
            remaining = quantum;
            current = cpu;
        }

        // if current process isnt idle 
        if(current != cpu && remaining == 0){
            remaining = quantum;
            if(!readyQueue.empty()){ 
                readyQueue.push_back(current);
                current = cpu;
            }
        }

        // while an incoming processes arrival_time time is less than or equal to current time - add to ready queue and remove from job queue
        while(jobQueue.size() > 0 && processes[jobQueue[0]].arrival_time <= curr_time){ // if arrival_time time is current or already passed
            readyQueue.push_back(jobQueue[0]); 
            jobQueue.erase(jobQueue.begin()); 
        }


        // if current process is idle and the cpu isnt empty - then start that process
        if(current == cpu && !readyQueue.empty()){
            current = readyQueue[0];
            remaining = quantum;
            if(((int)seq.size() == 0 || (seq.back() != current)) && (int)seq.size() < max_seq_len){
                seq.push_back(current);
            }
            readyQueue.erase(readyQueue.begin()); // remove process from ready queue
            if(started[current] == false){ 
                processes[current].start_time = curr_time;
                started[current] = true;
            }  
        }

        // if the current process is not idle and is the last job - finish it
        if(current != cpu && jobs_remaining == 1){
            curr_time = curr_time + remaining_bursts[current];
            processes[current].finish_time = curr_time;
            jobs_remaining--;
            continue;
        }


        // if the current process isnt idle & it has more time left in its time slice & the ready queue is empty - 
        if(current != cpu && remaining > 0 && readyQueue.empty()){
            int64_t amount = processes[jobQueue[0]].arrival_time - curr_time; // amount of time before the next process

            if(amount <= remaining && amount <= remaining_bursts[current]){
                remaining_bursts[current] = remaining_bursts[current] - amount;
                remaining = remaining - amount;
                curr_time = processes[jobQueue[0]].arrival_time;
            }else if(amount <= remaining && amount > remaining_bursts[current]){
                curr_time = curr_time + remaining_bursts[current];
                remaining_bursts[current] = 0;
            }else if(amount > remaining){
                remaining_bursts[current] = remaining_bursts[current] - remaining;
                curr_time = curr_time + remaining;
                remaining = 0;
            }
            continue;
        }


        if(current != cpu && remaining > 0 && !readyQueue.empty()){
            // check for pattern
            bool check = true;
            std::vector<int> tempRQ = readyQueue;
            tempRQ.push_back(current);
            int index = seq.size() - tempRQ.size();
            if(index < 0){check = false;}
            for(int i = 0; i < (int)tempRQ.size(); i++){
                if(seq[index] != tempRQ[i]){
                    check = false;
                }
                index++;
            }

            if(check){
                int64_t min = remaining_bursts[tempRQ[0]] / (quantum);
                for(int j = 0; j < (int)tempRQ.size(); j++){
                    int64_t n = remaining_bursts[tempRQ[j]] / (quantum);
    
                    if(j == 0){ min = n;}
                    if(tempRQ[j] == current){
                        n--;
                        if(n <= 0){min = 0;}
                    }
                    if(n >= 0){
                        if(n < min){ min = n;}
                    }
                }

                int64_t totalTime = min * quantum * ((int)tempRQ.size());

                if(!jobQueue.empty()){
                    if(curr_time + totalTime > processes[jobQueue[0]].arrival_time && min!=0){
                        min = (processes[jobQueue[0]].arrival_time - curr_time) / (quantum * (int)tempRQ.size());
                        totalTime = min * quantum * ((int)tempRQ.size());
                    }
                }

                if(min <= 1){
                    if(remaining_bursts[current] > remaining){
                        remaining_bursts[current] = remaining_bursts[current] - remaining;
                        curr_time = curr_time + remaining;
                        remaining = 0;
                    }else{
                        curr_time = curr_time + remaining_bursts[current];
                        remaining_bursts[current] = 0;
                    }

                    while(jobQueue.size() > 0 && processes[jobQueue[0]].arrival_time < curr_time){ // if arrival_time time of process has passed during time jump
                        readyQueue.push_back(jobQueue[0]); 
                        jobQueue.erase(jobQueue.begin());
                    }
                    continue;
                }

                for(int64_t n = min; n > 0 && (int)seq.size() < max_seq_len; n--){ // add pattern to the seq up to N times
                    for(int j = 0; j < (int)tempRQ.size() ; j++){ 
                        if((int)seq.size() < max_seq_len){
                            seq.push_back(tempRQ[j]);
                        }
                    }
                }

                for(int j = 0; j < (int)tempRQ.size(); j++){ // decrease total remaining for each process
                    remaining_bursts[tempRQ[j]] = remaining_bursts[tempRQ[j]] - (min * quantum);
                }
                curr_time = curr_time + totalTime; 
            
            }else{
                if(remaining_bursts[current] > remaining){
                    remaining_bursts[current] = remaining_bursts[current] - remaining;
                    curr_time = curr_time + remaining;
                    remaining = 0;
                }else{
                    curr_time = curr_time + remaining_bursts[current];
                    remaining_bursts[current] = 0;
                }
            }

            while(jobQueue.size() > 0 && processes[jobQueue[0]].arrival_time < curr_time){ // if arrival_time time of process has passed during time jump
                readyQueue.push_back(jobQueue[0]); 
                jobQueue.erase(jobQueue.begin()); 
            }
            continue;
        }
       
        // if current process is idle and the ready queue is empty
        if(current == cpu && readyQueue.empty()){
            if(jobs_remaining != 0){ 
                if(((int)seq.size() == 0 || (seq.back() != current)) && (int)seq.size() < max_seq_len){
                    seq.push_back(cpu); 
                }
                curr_time = processes[jobQueue[0]].arrival_time;
            }
            continue;
        }
    }
}
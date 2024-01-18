#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

const int num_astronomers = 10;
const int num_asymmetric = 3;  // Number of asymmetric astronomers
const int avg_eat_time = 1;
const int max_wait_time = 2;

std::vector<std::mutex> forks(num_astronomers);
std::vector<std::condition_variable> fork_cv(num_astronomers);

std::vector<std::mutex> eating_mutexes(num_astronomers);

void think() {
    // Implement philosopher thinking logic here
    // You can use this function to represent a philosopher thinking
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void eat() {
    // Implement philosopher eating logic here
    // You can use this function to represent a philosopher eating
    std::this_thread::sleep_for(std::chrono::seconds(avg_eat_time));
}

void asymmetric_astronomer(int id, const std::vector<bool>& order) {
    while (true) {
        think();

        int left_fork = (id + 1) % num_astronomers;  // Change the order of fork acquisition
        int right_fork = id;

        std::unique_lock<std::mutex> right_lock(forks[right_fork]);
        std::cout << "Astronomer " << id << " picked up right" << std::endl;

        // Wait for 1 second after picking up the right chopstick
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::unique_lock<std::mutex> left_lock(forks[left_fork]);
        std::cout << "Astronomer " << id << " picked up left" << std::endl;

        if (left_lock.owns_lock() && right_lock.owns_lock()) {
            eat();
            std::cout << "Astronomer " << id << " is eating." << std::endl;
        } else {
            left_lock.unlock();
            right_lock.unlock();
            std::cout << "Astronomer " << id << " dropped chopstick" << std::endl;
        }
    }
}


void symmetric_astronomer(int id) {
    while (true) {
        think();

        int left_fork = id;
        int right_fork = (id + 1) % num_astronomers;

        std::unique_lock<std::mutex> left_lock(forks[left_fork]);
        std::unique_lock<std::mutex> right_lock(forks[right_fork]);

        eat();
        std::cout << "Astronomer " << id << " is eating." << std::endl;

        left_lock.unlock();
        right_lock.unlock();
    }
}


std::vector<bool> place_astronomers(int num_astronomers, int num_asymmetric){
    // place both types of astronomers in a random order
    // asymmetric astronomers are represented by true
    // symmetric astronomers are represented by false

    std::vector<bool> order;
    for (int i = 0; i < num_asymmetric; i++) {
        order.push_back(true);
    }
    for (int i = num_asymmetric; i < num_astronomers; i++) {
        order.push_back(false);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(order.begin(), order.end(), gen);
    
    return order;
}

int main() {
    std::vector<bool> philosopher_order = place_astronomers(num_astronomers, num_asymmetric);

    // Initialize forks

    // Start philosopher threads
    std::vector<std::thread> philosopher_threads;
    for (int i = 0; i < num_astronomers; i++) {
        if (philosopher_order[i]) {
            philosopher_threads.emplace_back(asymmetric_astronomer, i, std::ref(philosopher_order));
        } else {
            philosopher_threads.emplace_back(symmetric_astronomer, i);
        }
    }

    // Call join on philosopher threads
    for (int i = 0; i < num_astronomers; i++) {
        philosopher_threads[i].join();
    }

    return 0;
}

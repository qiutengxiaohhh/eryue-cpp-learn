#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <functional>

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <memory>


//普通的线程池
/*
class MyThreadPool {

private:
    std::vector<std::thread> thread_pool;
    std::queue<std::function<void()>> task_queue;

    std::mutex mtx;
    std::condition_variable condition;

    bool stop;
public:

    MyThreadPool(int thread_num) : stop(false) {

        //添加线程
        for (int i = 0; i < thread_num; i++) {
            thread_pool.emplace_back([this]{
                while (1) {
                    std::unique_lock<std::mutex> lock(mtx);

                    //判断线程等待条件
                    condition.wait(lock , [this](){
                        return !task_queue.empty() || stop;
                    });

                    //线程池停止
                    if(stop && task_queue.empty()) {
                        return;
                    }

                    //move拷贝任务函数
                    if(!task_queue.empty()) {
                        std::function<void()> task(std::move(task_queue.front()));
                        task_queue.pop();
                        task();
                    }


                    lock.unlock();

                }
            });
        }

    }

    ~MyThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }


        //通知所有线程去完成任务队列所有任务
        condition.notify_all();
        for (auto &t: thread_pool) {
            t.join();
        }
    }

    //模板使用
    template<class F,class... Args>
    void addTask(F &&f , Args&&... args) {
        std::function<void()> task =
                std::bind(std::forward<F>(f) ,std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(mtx);
            task_queue.emplace(std::move(task));
        }

        condition.notify_one();
    }
};
*/

//使用互斥锁与共享指针的双层检测锁单例模式
/*class MyThreadPool {

private:
    std::vector<std::thread> thread_pool;
    std::queue<std::function<void()>> task_queue;

    std::condition_variable condition;

    static std::shared_ptr<MyThreadPool> myThreadPool_instance;
    static std::mutex mtx;
    bool stop;


    MyThreadPool(int thread_num) : stop(false){
        init(thread_num);
    };
    //初始化
    void init(int thread_num){
        //添加线程
        for (int i = 0; i < thread_num; i++) {
            thread_pool.emplace_back([this]{
                while (1) {
                    std::unique_lock<std::mutex> lock(mtx);

                    //判断线程等待条件
                    condition.wait(lock , [this](){
                        return !task_queue.empty() || stop;
                    });

                    //线程池停止
                    if(stop && task_queue.empty()) {
                        return;
                    }

                    //move拷贝任务函数
                    if(!task_queue.empty()) {
                        std::function<void()> task(std::move(task_queue.front()));
                        task_queue.pop();
                        lock.unlock();
                        task();

                    }


                }
            });
        }

    }



public:
    MyThreadPool(MyThreadPool&) = delete;
    MyThreadPool& operator=(const MyThreadPool&) = delete;

    static std::shared_ptr<MyThreadPool> getInstance(int thread_num){

        //双层检测锁
        if(myThreadPool_instance == nullptr) {
            std::lock_guard<std::mutex> lock(mtx);
            if(myThreadPool_instance== nullptr) {
                myThreadPool_instance =std::shared_ptr<MyThreadPool>(new MyThreadPool(thread_num));
            }
        }

        return myThreadPool_instance;
    }



    ~MyThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }


        //通知所有线程去完成任务队列所有任务
        condition.notify_all();
        for (auto &t: thread_pool) {
            t.join();
        }
    }

    //模板使用
    template<class F,class... Args>
    void addTask(F &&f , Args&&... args) {
        std::function<void()> task =
                std::bind(std::forward<F>(f) ,std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(mtx);
            task_queue.emplace(std::move(task));
        }

        condition.notify_one();
    }
};
std::shared_ptr<MyThreadPool> MyThreadPool::myThreadPool_instance = nullptr;
std::mutex MyThreadPool::mtx;*/

//魔法静态变量模式
class MyThreadPool {

private:
    std::vector<std::thread> thread_pool;
    std::queue<std::function<void()>> task_queue;

    std::condition_variable condition;


    std::mutex mtx;
    bool stop;


    MyThreadPool(int thread_num) : stop(false){
        init(thread_num);
    };
    //初始化
    void init(int thread_num){
        //添加线程
        for (int i = 0; i < thread_num; i++) {
            thread_pool.emplace_back([this]{
                while (1) {
                    std::unique_lock<std::mutex> lock(mtx);

                    //判断线程等待条件
                    condition.wait(lock , [this](){
                        return !task_queue.empty() || stop;
                    });

                    //线程池停止
                    if(stop && task_queue.empty()) {
                        return;
                    }

                    //move拷贝任务函数

                    std::function<void()> task(std::move(task_queue.front()));
                    task_queue.pop();
                    lock.unlock();
                    task();



                }
            });
        }

    }



public:
    MyThreadPool(MyThreadPool&) = delete;
    MyThreadPool& operator=(const MyThreadPool&) = delete;

    //局部静态变量
    static MyThreadPool& getInstance(int thread_num){

        static MyThreadPool myThreadPool_instance(thread_num);

        return myThreadPool_instance;
    }



    ~MyThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }


        //通知所有线程去完成任务队列所有任务
        condition.notify_all();
        for (auto &t: thread_pool) {
            t.join();
        }
    }

    //模板使用
    template<class F,class... Args>
    void addTask(F &&f , Args&&... args) {
        std::function<void()> task =
                std::bind(std::forward<F>(f) ,std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(mtx);
            task_queue.emplace(std::move(task));
        }

        condition.notify_one();
    }
};


int main() {
    MyThreadPool& myThreadPool = MyThreadPool::getInstance(4);

    for (int i = 0; i < 10; i++) {
        myThreadPool.addTask([i](){
            std::cout<<"task:"<<i<<" is running"<<std::endl;
            std::this_thread::sleep_for(std::chrono::seconds (1));
            std::cout<<"task:"<<i<<" is over"<<std::endl;
        });
    }

    return 0;
}

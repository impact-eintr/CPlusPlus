#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void thread1(){
    cout << "thread 01:" << endl;
    for(int i = 0; i < 10; i++){
        cout << "t1:" << i << endl;
    }
}

void thread2(){
    cout << "thread 02:" << endl;
    for(int i = 10; i > 0; i--){
        cout << "t2:" <<  i << endl;
    }
}

int main(int argc, char** argv){

    thread task1(thread1);
    thread task2(thread2);

    task1.join();
    task2.join();

    sleep(1);
    return 0;
}

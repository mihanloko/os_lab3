#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <thread>
#include <vector>

using namespace std;

struct MsgData {
    int key;
    char str[50];

    string toString() {
        return "MsgData{key: " + to_string(key) + ", str: " + string(str) + "}";
    }
};

struct Msg {
    long type;
    MsgData data;
};

std::string gen_random(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

int serverFunction(int num) {
    key_t MyIPC_Key;
    int MessageID, Repeat;
    Repeat = 1;
    MyIPC_Key = 12345; /*ключ доступа к ресурсу */
// получаем идентификатор очереди сообщений
    if ((MessageID = msgget(MyIPC_Key, 0777 | IPC_CREAT)) < 0) {
        fprintf(stderr, "Server %d can't Create Queue\n", num);
        return 1;
    }
    Msg message{};
    while (Repeat) {
        long size = msgrcv(MessageID, &message, sizeof(MsgData), 1, 0);

        if ( size < 0 ) {
            printf("Сервер %d закончил работу\n", num);
            Repeat = 0;
        } else {
            printf("Полученная строка сервером %d: %s, size %ld\n", num, message.data.toString().c_str(), size);
            sleep(message.data.key % 4);
        }
    }
    return 0;
}


int clientFunction(int num) {
    key_t MyIPC_Key;
    int MessageID, Repeat;
    Repeat = 1;
    MyIPC_Key = 12345; /*ключ доступа к ресурсу */
// создаём очередь сообщений
    if ((MessageID = msgget(MyIPC_Key, 0777 | IPC_CREAT)) < 0) {
        fprintf(stderr, "Client %d can't Create Queue\n", num);
        return 1;
    }
    Msg message{};
    for (int i = 0; i < 10; i++) {
        message.type = 1;
        strcpy(message.data.str, gen_random(num * 3).c_str());
        message.data.key = rand() % 100;
        msgsnd(MessageID, &message, sizeof(MsgData), 0);
        sleep(message.data.key % 2);
    }
    return 0;
}

int main() {
    srand(time(0));
    vector<thread> clients;
    vector<thread> servers;
    for (int i = 0; i < 3; i++) {
        clients.emplace_back(clientFunction, i + 1);
        servers.emplace_back(serverFunction, i + 1);
    }
    for (auto &item : clients) {
        item.join();
    }
    int MessageID;
    key_t MyIPC_Key = 12345; /*ключ доступа к ресурсу */
    if ((MessageID = msgget(MyIPC_Key, 0777)) < 0) {
        fprintf(stderr, "Main can't Create Queue\n");
        return 1;
    } else {
        msqid_ds t{};
        while (true) {
            int stat = msgctl(MessageID, IPC_STAT, &t);
            if (stat >= 0 && t.msg_qnum == 0) {
                msgctl(MessageID, IPC_RMID, nullptr);
                printf("Queue stopped\n");
                break;
            } else {
                sleep(1);
            }
        }
    }
    for (auto &item : servers) {
        item.join();
    }
    return 0;
}
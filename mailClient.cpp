#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <fstream>
// #include <mimetic/mimetic.h>
// #include <mimetic/mimeentity.h>
// #include <mimetic/utils.h>
#include <filesystem>


#define bufferSize 1024

using namespace std;
using namespace filesystem;
// using namespace mimetic;

struct mailContent
{
    string from;
    string to;
    string cc;
    string bcc;
    string subject;
    string content;
    // Them file
    vector<string> fileName;
    vector<string> filePath;
    vector<string> fileData;
};

struct config
{
    string uname;
    string pass;
    string mailServer;
    int smtp;
    int pop3;
    int autoLoad;
};

config readConfig()
{
    config con;
    fstream file;
    file.open("config.txt", ios::in);
    getline(file, con.uname);
    getline(file, con.pass);
    getline(file, con.mailServer);
    string tmp;
    getline(file, tmp);
    con.smtp = stoi(tmp);
    getline(file, tmp);
    con.pop3 = stoi(tmp);
    getline(file, tmp);
    con.autoLoad = stoi(tmp);
    file.close();
    return con;
}

int choiceUI()
{
    int choice = 0;
    cout << "Vui long chon Menu:\n";
    cout << "1. Gui mail\n";
    cout << "2. Xem cac mail da nhan\n";
    cout << "0. Thoat\n";
    cout << "Lua chon: ";
    cin >> choice;
    cout << "Ban chon: " << choice << endl;
    return choice;
}

bool serverReply(int client_fd)
{
    char buffer[1024];
    int valRead;

    if ((valRead = recv(client_fd, buffer, sizeof(buffer), 0)) < 0)
    {
        cout << "Fail to recv from server\n";
        return 0;
    }
    // else
    // {
    //     cout << "Server reply: ";
    //     for (int i = 0; i < valRead; i++)
    //     {
    //         cout << buffer[i];
    //     }
    //     cout << endl;
    // }
    memset(buffer, 0, sizeof(buffer));
    return 1;
}

string serverReplyStr(int client_fd)
{
    char buffer[1024];
    int valRead;
    string rep = "";

    if ((valRead = recv(client_fd, buffer, sizeof(buffer), 0)) < 0)
    {
        cout << "Fail to recv from server\n";
    }
    else
    {
        // cout << "Server reply: ";
        for (int i = 0; i < valRead; i++)
        {
            // cout << buffer[i];
            rep += buffer[i];
        }
        // cout << endl;
    }
    memset(buffer, 0, sizeof(buffer));
    return rep;
}

string getString()
{
    string a;
    int c;
    do 
    {
        c = getchar();
        if (c != '\n')
            a.push_back(c);
        else
            break;
    }
    while(1);
    return a;
}

int getFileSize(string filePath)
{
    int size;
    fstream file;
    file.open(filePath, ios::in | ios::binary | ios::ate);
    size = static_cast<int>(file.tellg());
    file.close();
    return size;
}

mailContent writeMail(config con)
{
    mailContent a;
    a.from = con.uname; // Doc file config
    // cin.ignore();
    cout << "Day la thong tin soan mail (nhan Enter neu khong muon nhap gi hoac muon ket thuc viec nhap)\n";
    cin.ignore();
    cout << "To: ";
    a.to = getString();
    cout << "CC: ";
    a.cc = getString();
    cout << "BCC: ";
    a.bcc = getString();
    cout << "Subject: ";
    a.subject = getString();
    cout << "Content: ";
    a.content = getString();
    // Gui kem file
    string fileChoice;
    // cin.ignore();
    cout << "Ban co muon dinh kem file khong (1. Co | 0. Khong): \n";
    // getline(cin, fileChoice);
    // cin.ignore();
    cin >> fileChoice;
    cin.ignore();
    if (fileChoice == "1")
    {
        do
        {
            string tmp;
            // string fileName;
            cout << "Nhap ten file: ";
            // a.fileName = getString();
            tmp = getString();
            a.fileName.push_back(tmp);
            // cin.ignore();
            string tmp1;
            cout << "Nhap duong dan cua file: ";
            // a.filePath = getString();
            tmp1 = getString();
            a.filePath.push_back(tmp1);
            // cin.ignore();

            // Check file size
            if (getFileSize(tmp1) > 3000000)
            {
                cout << "File > 3MB. Hay chon file khac\n";
            }

            int more = 0;
            cout << "Ban co muon gui them file (1. Co | 0. Khong): ";
            cin >> more;
            cin.ignore();
            if (more != 1)
                break;
            // else
            //     break;
        } while (1);
        

        for (int i = 0; i < a.fileName.size(); i++)
        {
            fstream file;
            file.open(a.filePath[i], ios::in);
            string d;
            while (!file.eof())
            {
                string tmp;
                getline(file, tmp);
                d += tmp + '\n';
            }
            d += "-----";
            file.close();
            cout << "My File\n" << d << endl;
            a.fileData.push_back(d);
        }

        // cout << "Data: \n";
        // cout << a.fileData << endl;
    }
    return a;
}

bool sendInitData(mailContent a, int client_fd)
{
    bool status;
    string sendMsg;

    sendMsg = "MAIL FROM: <" + a.from + ">\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    sendMsg = "RCPT TO: <" + a.to + ">\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    if (a.cc != "")
    {
        sendMsg = "RCPT TO: <" + a.cc + ">\r\n";
        send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
        status = serverReply(client_fd);
        if (status == 0)
            return 0;
    }

    if (a.bcc != "")
    {
        sendMsg = "RCPT TO: <" + a.bcc + ">\r\n";
        send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
        status = serverReply(client_fd);
        if (status == 0)
            return 0;
    }

    return 1;
}

string getTime()
{
    time_t current_time = time(nullptr);
    struct tm *local_time = localtime(&current_time);
    string a = asctime(local_time);
    return a;
}

bool sendData(mailContent a, int client_fd)
{
    bool status;

    string sendMsg = "DATA\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;
    
    string dataa = "";
    dataa += "Date: " + getTime();
    dataa += "From: " + a.from + "\r\n";
    dataa += "To: <" + a.to + ">\r\n";
    if (a.cc != "")
        dataa += "CC: <" + a.cc + ">\r\n";
    // if (a.bcc != "")
    //     dataa += "BCC: <" + a.bcc + ">\r\n";
    dataa += "Subject: " + a.subject + "\r\n\r\n";

    // Content part
    dataa += a.content + "\r\n\r\n";

    // File attachment part
    for (int i = 0; i < a.fileName.size(); i++)
    {
        if (a.fileName[i] != "")
        {
            dataa += a.fileName[i] + "\r\n";
            dataa += a.fileData[i] + "\r\n\r\n";
        }
    }


    dataa += ".\r\n";

    sendMsg = dataa;
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    return 1;
}


void smtp(config con)
{
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Create socket failed \n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(con.smtp);
    serv_addr.sin_addr.s_addr = inet_addr(con.mailServer.c_str());
    // cout << con.mailServer.c_str() << endl;

    bool status = 1;

    // =======================================
    if (connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Connection failed \n";
        close(client_fd);
        return;
    }
    else
    {
        status = serverReply(client_fd);
        if (status == 0)
            return;
    }

    // =======================================
    // Send hello
    string sendMsg = "EHLO Khoi\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Enter mail
    mailContent a = writeMail(con);
    status = sendInitData(a, client_fd);
    if (status == 0)
        return;
    // Send Data
    bool sd =  sendData(a, client_fd);
    if (sd == 1)
        cout << "Da gui thu\n";
    else
        cout << "Gui that bai\n";
    // sendDataWithMIME(a, client_fd);

    // =======================================
    // Quit
    sendMsg = "QUIT\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return;

    close(client_fd);
}

// =================================

struct listMail
{
    string stt;
    int bytes;
};

vector<listMail> readLIST(string a)
{
    vector<listMail> li;
    stringstream ss(a);
    string tmp;
    getline(ss, tmp);
    // cout << tmp << endl;
    while(1)
    {
        listMail l;
        getline(ss, tmp);
        // cout << tmp << endl;
        if (tmp == ".\r")
        {
            // cout << "Vao r\n";
            break;
        }
        stringstream sss(tmp);
        getline(sss, tmp, ' ');
        l.stt = tmp;
        getline(sss, tmp);
        // cout << tmp << endl;
        l.bytes = stoi(tmp);
        li.push_back(l);
    }
    // cout << "Ra roi\n";
    return li;
}

vector<string> readDataPOP3(string a)
{
    // cout << a << endl;
    vector<string> parts;
    int start = 0;
    while (1)
    {
        int pos = a.find("\r\n\r\n", start);
        string tmp1 = a.substr(start, pos - start);
        // cout << tmp1 << endl << "==============\n";
        if (tmp1[0] == '.')
            break;
        parts.push_back(tmp1);
        // string tmp = a.substr(pos - start + 4);
        start = pos + 4;
    }
    return parts;
}

struct mailParts
{
    string header;
    string content;
    vector<string> fileName;
    vector<string> filePath;
};

vector<string> readFileAttach(string a)
{
    vector<string> v;
    // stringstream ss(a);
    // string tmp;

    // getline(ss, tmp);
    // v.push_back(tmp);

    // tmp = ss.str();
    // v.push_back(tmp);

    int pos = a.find("\n");
    v.push_back(a.substr(0, pos));
    v.push_back(a.substr(pos + 1));

    return v;
}

void writeFileAttach(string fileName, string data, string mailAccount)
{
    fstream file;
    file.open(mailAccount + "/files/" + fileName, ios::out);
    file << data.substr(0, data.size() - 5);
    file.close();
}

void writeMailToFolder(mailParts mp, string mailAccount, string idMail, string mailType)
{
    // cout << mp.fileName << endl;
    fstream file;
    file.open(mailAccount + "/" + mailType + "/" + idMail + ".txt", ios::out);
    file << mp.header << "\r\n---\r\n" << mp.content << "\r\n";
    for (int i = 0; i < mp.fileName.size(); i++)
    {
        if (mp.fileName[i] != "" || mp.fileName[i] != "-----")
        {
            string tmp = " is attach";
            string tmp2 = mp.fileName[i].insert(mp.fileName[i].length() - 1, tmp);
            // cout << "aaaaaaaaaaaaaaaaaaaaaaaa " << tmp2 << endl;
            mp.fileName[i] = tmp2;
            file << "\r\n";
        }
        file << mp.fileName[i] << "\r\n" << ".\r\n";
    }
    file.close();

    fstream file2;
    file.open(mailAccount + "/" + mailType + "/status.txt", ios::out | ios::app);
    file << idMail << endl;
    file << "0\n";
    file.close();
}

mailParts readDataPop3FromMail(vector<string> parts, string mailAccount)
{
    mailParts mp;

    mp.header = parts[0];
    mp.content = parts[1];

    if (parts.size() > 2)
    {
        for (int i = 2; i < parts.size(); i ++)
        {
            // cout << "HAHAHAHAHA " << parts[2] << endl;
            vector<string> v = readFileAttach(parts[i]);
            mp.fileName.push_back(v[0]);
            if (v[0] != "-----")
                writeFileAttach(v[0], v[1], mailAccount);
            mp.filePath.push_back(mailAccount + "/files/" + v[0]);
        }
    }
    
    return mp;
}

void createMailBoxFolder(string mailAccount)
{
    if (create_directory(mailAccount) != 0)
    {
        fstream file;
        file.open(mailAccount + "/list.txt", ios::out);
        if (!file.is_open())
            cout << "Khong the tao file\n";
        file.close(); 
    }


    if (create_directory(mailAccount + "/inbox"))
        cout << "Tao thu muc inbox thanh cong\n";

    if (create_directory(mailAccount + "/project"))
        cout << "Tao thu muc project thanh cong\n";

    if (create_directory(mailAccount + "/important"))
        cout << "Tao thu muc important thanh cong\n";

    if (create_directory(mailAccount + "/spam"))
        cout << "Tao thu muc spam thanh cong\n";

    if (create_directory(mailAccount + "/work"))
        cout << "Tao thu muc work thanh cong\n";

    if (create_directory(mailAccount + "/files"))
        cout << "Tao thu muc files thanh cong\n";

    
}

struct mailStatus
{
    string id;
    string status;
};

vector<mailStatus> readStatus(string mailAccount, string mailType)
{
    vector<mailStatus> ms;
    fstream file;
    file.open(mailAccount + "/" + mailType + "/" + "status.txt" , ios::in);
    if (!file.is_open())
    {
        cout << "No\n";
        return ms;
    }
    do
    {
        mailStatus mss;
        getline(file, mss.id);
        getline(file, mss.status);
        // cout << mss.id << " --- " << mss.status << endl;
        if (mss.id != "")
            ms.push_back(mss);
    }
    while(!file.eof());
    file.close();
    return ms;
}

void writeStatus(string mailAccount, string mailType, vector<mailStatus> ms)
{
    fstream file;
    file.open(mailAccount + "/" + mailType + "/" + "status.txt" , ios::out);
    for (int i = 0; i < ms.size(); i++)
    {
        file << ms[i].id << endl;
        file << ms[i].status << endl;
    }
    file.close();
}

vector<string> readUserMails(string mailAccount)
{
    vector<string> mails;
    fstream file;
    file.open(mailAccount + "/list.txt", ios::in);
    while(!file.eof())
    {
        string tmp;
        getline(file, tmp);
        if (tmp != "")
        {
            mails.push_back(tmp);
        }
    }
    file.close();
    return mails;
}

void writeUserMails(string mailAccount, vector<string> mails)
{
    fstream file;
    file.open(mailAccount + "/list.txt", ios::out);
    for (int i = 0; i < mails.size(); i++)
    {
        file << mails[i] << endl;
    }
    file.close();
}

vector<string> getNewMailID(vector <string> userMails, vector<listMail> lm)
{
    vector<string> newMailID;
    for (int i = userMails.size(); i < lm.size(); i++)
    {
        newMailID.push_back(lm[i].stt);
    }
    return newMailID;
}

vector<string> getMailData(string mailAccount, string type, string id)
{
    vector<string> p;
    // cout << "Here\n";
    string dataa = "";
    fstream file;
    file.open(mailAccount + "/" + type + "/" + id + ".txt", ios::in);
    if (!file.is_open())
    {
        cout << "NO\n";
        return p;
    }
    while (!file.eof())
    {
        string tmp;
        getline(file, tmp);
        dataa += tmp + "\r\n";
    }
    file.close();
    // cout << dataa << endl;

    string tmp;
    int pos = dataa.find("\r\n");
    tmp = dataa.substr(pos + 2);
    // cout << "---\n";
    // cout << tmp << endl;

    int pos2 = tmp.find("---");
    p.push_back(tmp.substr(0, pos2));
    p.push_back(tmp.substr(0, tmp.size() - 5));
    // cout << "+++++++++++=\n";
    // cout << p[0] << "\nhahaha\n" << p[1] << endl;
    return p;
}

vector<vector<string>> getAllMailData(vector<mailStatus> ms, string type, string mailAccount)
{
    vector<vector<string>> mp;
    for (int i = 0; i < ms.size(); i++)
    {
        vector<string> mps = getMailData(mailAccount, type, ms[i].id);
        mp.push_back(mps);
        continue;
    }
    return mp;
}

vector<vector<string>> printMailBox(vector<mailStatus> ms, string type, string mailAccount)
{
    vector<vector<string>> mp = getAllMailData(ms, type, mailAccount);
    for (int i = 0; i < mp.size(); i++)
    {
        cout << "---------------" << i + 1 << "---------------";
        if (ms[i].status == "0")
            cout << "(chua doc)";
        cout << endl;
        cout << mp[i][0] << endl;
        cout << "-------------------------------\n";
    }
    return mp;
}

void readMail(int choice, string dataa)
{
    cout << "\nNoi dung mail thu " << choice << endl;
    cout << "==============================\n";
    cout << dataa << endl;
    cout << "==============================\n";
}

string chooseFolderToReadMail()
{
    int choice = 0;
    cout << "Cac Folder trong Mailbox: \n";
    cout << "1. Inbox\n2. Project\n3. Important\n4. Work\n5. Spam\n";
    cout << "Hay chon Folder (Chon 0 de thoat): ";
    cin >> choice;
    if (choice == 1)
        return "inbox";
    if (choice == 2)
        return "project";
    if (choice == 3)
        return "important";
    if (choice == 4)
        return "work";
    if (choice == 5)
        return "spam";
    return "";
}

void choiceMailToRead(string userMail)
{
    string mailType = chooseFolderToReadMail();
    if (mailType == "")
        return;
    vector<mailStatus> ms = readStatus(userMail, mailType);
    cout << "Day la danh sach mail trong folder " << mailType << endl;
    vector<vector<string>> mp = printMailBox(ms, mailType, userMail);

    int choice = 0;
    do
    {
        cout << "Hay chon mail muon doc (chon 0 neu muon huy): ";
        cin.ignore();
        cin >> choice;
        if (choice == 0)
            return;
        cout << "Ban chon thu: " << choice << endl;
    }
    while(choice > ms.size());
    readMail(choice, mp[choice - 1][1]);

    // Update state
    ms[choice - 1].status = "1";
    writeStatus(userMail, mailType, ms);

    return;
}

string getMailTypeFromSender(string header)
{
    int pos = header.find("<");
    int pos1 = header.find(">\r");
    string sender = header.substr(pos + 1, pos1 - pos - 1);
    return sender;
}

string getMailTypeFromSubject(string header)
{
    int pos = header.find("Subject: ");
    string subject = header.substr(pos + 9);
    return subject;
}

string getMailType(string header, string content)
{
    // cout << "Content: " << content << endl;
    string sender = getMailTypeFromSender(header);
    if (sender == "sep@hcmus.vn")
        return "project";

    string subject = getMailTypeFromSubject(header);
    if (subject.find("urgent") != string::npos || subject.find("ASAP") != string::npos)
        return "important";
    
    if (content.find("work") != string::npos || content.find("meeting") != string::npos)
        return "work";

    if (subject.find("hack") != string::npos || subject.find("crack") != string::npos || content.find("hack") != string::npos || content.find("crack") != string::npos)
        return "spam";

    return "inbox";
}

void pop3(config con)
{
    string mailAccount = "u0@hcmus.vn";
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Create socket failed \n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(con.pop3);
    serv_addr.sin_addr.s_addr = inet_addr(con.mailServer.c_str());

    // Create mail box for user
    createMailBoxFolder(mailAccount);

    bool status = 1;
    // =======================================
    if (connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Connection failed \n";
        close(client_fd);
        return;
    }
    else
    {
        status = serverReply(client_fd);
        if (status == 0)
            return;
    }

    // =======================================
    // Send CAPA
    string sendMsg = "CAPA\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send User info
    // cout << "Going here\n";
    sendMsg = "USER " + mailAccount + "\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    // cout << "End here\n";
    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send User password
    sendMsg = "PASS 333\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send STAT and LIST; DOC CHO NAY CAN THAN
    sendMsg = "STAT\r\n"; // Read from config file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    sendMsg = "LIST\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    string rep = serverReplyStr(client_fd);
    vector<listMail> li = readLIST(rep);

    // Phai di so voi cac mail da nhan roi moi chay RETR

    // =======================================
    // Send UIDL
    sendMsg = "UIDL\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send Request to get mail
    vector<string> userMails = readUserMails(mailAccount);
    vector<string> newMailID = getNewMailID(userMails, li);

    for (int i = 0; i < newMailID.size(); i++)
    {
        // cout << "Mail thu: " << newMailID[i] << endl;
        sendMsg = "RETR " + newMailID[i] + "\r\n"; // Read from configure file
        send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

        // =======================================
        // Receive new mail
        char buffer[bufferSize];
        // memset(buffer, 0, sizeof(buffer));
        int valRead;

        if ((valRead = recv(client_fd, buffer, bufferSize, 0)) < 0)
        {
            cout << "Fail to recv from server\n";
            sendMsg = "QUIT\r\n";
            send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
            return;
        }
        else
        {
            userMails.push_back(newMailID[i]);
            // cout << "Mail i: " << newMailID[i] << endl;
            // cout << "Start =============\n";


            vector<string> parts = readDataPOP3(buffer);
            // cout << "Mid ===============\n";
            // for (int i = 0; i < parts.size(); i++)
            // {
            //     cout << parts[i] << endl;
            //     cout << "++++++++++++++\n";
            // }

            // cout << "Im here 1\n";
            mailParts mp = readDataPop3FromMail(parts, mailAccount);

            string mailType = getMailType(mp.header, mp.content);
            // cout << "Your mail in " << mailType << endl;

            // cout << "Im here 2\n";
            writeMailToFolder(mp, mailAccount, newMailID[i], mailType);
        }
    }
    // cout << "Here\n";
    // for (int i = 0; i < userMails.size(); i++)
    // {
    //     cout << userMails[i] << endl;
    // }
    writeUserMails(mailAccount, userMails);
    sendMsg = "QUIT\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    // cout << "Im here 3\n";
    choiceMailToRead(mailAccount);
}

int main()
{
    // Load file config
    config con;
    con = readConfig();

    // UI
    int choice = 0;
    do
    {
        choice = choiceUI();
        cout << "======================\n";

        if (choice == 1)
        {
            smtp(con);
            cout << "======================\n";
        }
        else if (choice == 2)
        {
            pop3(con);
        }
        // else if (choice == 3)
        // {
        //     // cout << getFileSize("./config.txt");
        // }
    }
    while (choice != 0);
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <chrono>

using namespace std;

class Node {
public:
    char ch;
    int freq;
    bool isLeafNode;
    Node* left, * right;

    Node() {
    }

    Node(char ch, int freq) {
        this->ch = ch;
        this->freq = freq;
        this->left = nullptr;
        this->right = nullptr;
        this->isLeafNode = true;

    }

    Node(char ch, int freq, Node* left, Node* right) {
        this->ch = ch;
        this->freq = freq;
        this->left = left;
        this->right = right;
        this->isLeafNode = false;
    }
};

struct LessFreq {
    bool operator() (const Node* struct1, const Node* struct2) {
        return (struct1->freq < struct2->freq);
    }
};

int computeBits(map<char, int> freq, map<char, string> huffmanCode) {
    int totalBits = 0;
    for (pair<char, string> pair : huffmanCode) {
        totalBits += pair.second.length() * freq[pair.first];
    }
    return totalBits;
}

string getBinaryString(map<char, string> huffmanCode, string plaintext) {
    string binary;
    for (int i = 0; i < plaintext.size(); i++) {
        binary += huffmanCode[plaintext[i]];
    }
    return binary;
}

void getHuffmanCode(Node* root, string str, map<char, string>& huffmanCode) {
    if (!root) {
        return;
    }
    if (root->isLeafNode) {
        huffmanCode[root->ch] = str;
    }
    else {
        getHuffmanCode(root->left, str + "0", huffmanCode);
        getHuffmanCode(root->right, str + "1", huffmanCode);
    }
}

vector<Node*> nodesArray;
vector<Node*> nodesQueue;
void buildHuffmanTree(string plaintext, string filename)
{
    map<char, int> freq;
    for (int i = 0; i < plaintext.size(); i++) {
        freq[plaintext[i]]++;
    }

    for (pair<char, int> pair : freq) {
        Node newNode(pair.first, pair.second);
        nodesArray.push_back(new Node(pair.first, pair.second));
        Node* temp = nodesArray[nodesArray.size() - 1];
        nodesQueue.push_back(temp);
    }

    while (nodesQueue.size() != 1) {
        sort(nodesQueue.begin(), nodesQueue.end(), LessFreq());
        Node* left = nodesQueue[0];
        nodesQueue.erase(nodesQueue.begin());
        Node* right = nodesQueue[0];
        nodesQueue.erase(nodesQueue.begin());
        int sum = left->freq + right->freq;

        Node newNode('\0', sum, left, right);
        nodesArray.push_back(new Node('\0', sum, left, right));
        Node* temp = nodesArray[nodesArray.size() - 1];
        nodesQueue.push_back(temp);
    }

    Node* root = nodesQueue[0];

    map<char, string> huffmanCode;
    getHuffmanCode(root, "", huffmanCode);

    ofstream MyFile(filename + ".zip301");
    for (pair<char, string> pair : huffmanCode) {
        if (pair.first == '\n') {
            MyFile << pair.second << " newline" << endl;
        }
        else if (pair.first == ' ') {
            MyFile << pair.second << " space" << endl;
        }
        else if (pair.first == '\r') {
            MyFile << pair.second << " return" << endl;
        }
        else if (pair.first == '\t') {
            MyFile << pair.second << " tab" << endl;
        }
        else {
            MyFile << pair.second << " " << pair.first << endl;
        }
    }
    MyFile << "*****" << endl;
    int totalBytes = computeBits(freq, huffmanCode);
    MyFile << totalBytes << endl;

    string binary = getBinaryString(huffmanCode, plaintext);

    int getCount = binary.length() / 8;
    int lastSubstringLength = binary.length() % 8;
    int j = 0;
    for (int i = 0; i < getCount; i++) {
        char ch;
        int num = stoi(binary.substr(j, 8), 0, 2);
        ch = num;
        MyFile << ch;
        j += 8;
    }
    if (lastSubstringLength > 0) {
        int neededZero = 8 - lastSubstringLength;
        string lastBinary = binary.substr(j, lastSubstringLength);
        for (int i = 0; i < neededZero; i++) {
            lastBinary = lastBinary + "0";
        }
        char ch;
        int num = stoi(binary.substr(j, 8), 0, 2);
        ch = num;
        MyFile << ch;
    }
    cout << "Zip301 Generated.\n";
}

int main(int argc, char** argv)
{
    chrono::high_resolution_clock::time_point starttime = chrono::high_resolution_clock::now();

    if (argc != 2) {
        cout << "You need to include a filename on the command line.\n";
        return -1;
    }

    string filename = argv[1];
    fstream file(filename, ios::in);

    string delimiter = ".";
    string name = filename.substr(0, filename.find(delimiter));
    string extension = filename.substr(1, filename.find(delimiter));

    if (!file) {
        cout << "Error opening file: " << filename << endl;
        return -1;
    }
    string input;
    string file_contents;
    bool firstLine = true;
    while (!file.eof())
    {
        getline(file, input);
        if (!firstLine) {
            file_contents.push_back('\n');
        }
        else {
            firstLine = false;
        }
        file_contents += input;
    }

    buildHuffmanTree(file_contents, name);

    chrono::high_resolution_clock::time_point endtime = chrono::high_resolution_clock::now();
    chrono::duration<double> time = endtime - starttime;
    cout << time.count() << "s";

    file.close();
    return 0;
}

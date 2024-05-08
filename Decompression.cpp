#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <sstream>

using namespace std;

// Define a node for the trie structure
struct TrieNode {
    // Each node has a map of its children nodes
    unordered_map<char, unique_ptr<TrieNode>> children;
    // Flag to indicate if this node represents the end of a word (or in this case, a character code)
    bool isEndOfWord = false;
    // The character value associated with the end of a word node
    char value;
};

// A class to facilitate reading individual bits from a binary string
class BitReader {
public:
    explicit BitReader(const string& s) : data(s), bitPos(7), byteIndex(0) {}

    // Function to read the next bit in the sequence
    int readBit() {
        if (byteIndex >= data.size()) return -1; // End of data check
        int bit = (data[byteIndex] >> bitPos) & 1; // Extract the current bit
        bitPos--; // Move to the next bit position
        if (bitPos < 0) { // Reset bit position and move to the next byte when all bits in the current byte are read
            bitPos = 7;
            byteIndex++;
        }
        return bit;
    }

private:
    const string& data; // The binary string to read from
    int bitPos; // Current position within a byte (0-7)
    size_t byteIndex; // Current byte position in the string
};

// Function to insert a character code and its associated character into the trie
void insertTrie(TrieNode& root, const string& key, char value) {
    TrieNode* node = &root;
    for (char bit : key) { // Traverse the bit string
        if (node->children.find(bit) == node->children.end()) { // Create a new node if it doesn't exist
            node->children[bit] = make_unique<TrieNode>();
        }
        node = node->children[bit].get(); // Move to the next node
    }
    node->isEndOfWord = true; // Mark the end of the character code
    node->value = value; // Store the character value
}

// Function to search the trie and decode a character based on a sequence of bits
char searchTrie(TrieNode& root, BitReader& bitReader, int& bitsRead, int maxBits) {
    TrieNode* node = &root;
    while (true) {
        if (node->isEndOfWord) {
            return node->value; // Return the character if the end of a character code is reached
        }
        int bit = bitReader.readBit(); // Read the next bit
        if (bit == -1 || bitsRead == maxBits) return '\0'; // Check for end of data or max bits reached
        bitsRead++; // Increment the bits read counter
        char bitChar = bit + '0'; // Convert the bit to a character ('0' or '1')
        if (node->children.find(bitChar) == node->children.end()) {
            return '\0'; // Return null character if the bit pattern doesn't match any character code
        }
        node = node->children[bitChar].get(); // Move to the next node
    }
}

int main(int argc, char** argv) {
    auto starttime = chrono::high_resolution_clock::now(); // Start a timer to measure execution time

    
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << endl;
        return 1;
    }

    // Open the input file
    ifstream ifs(argv[1], ios::in | ios::binary);
    if (!ifs) { // Check for file open errors
        cerr << "Error opening file: " << argv[1] << endl;
        return 1;
    }

    // Read and store the character codes in the trie
    TrieNode root;
    while (true) {
        string str, ch;
        ifs >> str; // Read the bit string (character code)
        if (str == "*****") break; // Check for the delimiter marking the end of the character codes section
        ifs >> ch; // Read the character associated with the code
        // Convert special string representations to their actual character counterparts
        if (ch == "newline") ch = '\n';
        else if (ch == "return") ch = '\r';
        else if (ch == "tab") ch = '\t';
        else if (ch == "space") ch = ' ';
        insertTrie(root, str, ch.front()); // Insert the character code and character into the trie
    }

    // Read the number of bits that encode the file
    string junk;
    getline(ifs, junk); // Skip the remaining part of the line after "*****"
    getline(ifs, junk); // Read the number of bits
    int numOfBits = stoi(junk); // Convert the number of bits from string to integer

    // Read the binary encoded portion of the file
    stringstream buffer;
    buffer << ifs.rdbuf(); // Read the remaining file content into a buffer
    string charToDecode = buffer.str(); // Convert the buffer content into a string
    ifs.close(); // Close the input file

    // Construct the output filename by removing ".zip301" and appending "2.txt"
    string inputFilename(argv[1]);
    string baseFilename = inputFilename.substr(0, inputFilename.size() - 7); // Remove the last 7 characters (".zip301")
    string outputFilename = baseFilename + "2.txt"; // Append "2.txt" to the base filename

    // Open the output file
    ofstream ofs(outputFilename, ios::out | ios::binary);
    BitReader bitReader(charToDecode); // Initialize the bit reader with the binary encoded data
    int bitsRead = 0;

    // Decode the binary data and write the characters to the output file
    while (bitsRead < numOfBits) {
        char decodedChar = searchTrie(root, bitReader, bitsRead, numOfBits); // Decode the next character
        if (decodedChar == '\0') break; // Stop if the end of the encoded data is reached
        ofs << decodedChar; // Write the decoded character to the output file
    }

    ofs.close(); // Close the output file

    // Measure and print the execution time
    auto endtime = chrono::high_resolution_clock::now();
    chrono::duration<double> timeTaken = endtime - starttime;
    cout << "Decompression completed in " << timeTaken.count() << " seconds." << endl;

    return 0; // End of the program
}
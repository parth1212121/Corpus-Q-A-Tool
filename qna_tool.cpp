#include "qna_tool.h"

#include <assert.h>

#include <sstream>

using namespace std;

Trie *parse_csv(string csv_file) {
    Trie *t = new Trie();
    Trie *stemt = new Trie();
    ifstream csv;
    csv.open(csv_file);
    // ignore first line
    string junk;
    csv >> junk;
    while (!csv.eof()) {
        string word;
        long long freq;
        string separator;
        string line;
        csv >> line;
        if (line.empty()) {
            continue;
        }
        int i = 0;
        while (line[i] != ',') {
            word += line[i];
            i++;
        }
        i++;
        // cout << line.substr(i, line.size() - i) << "\n";
        freq = stoll(line.substr(i, line.size() - i));
        // cout << "word: " << word << " ";
        // cout << "freq: " << freq << "\n";
        t->set_freq(word, freq);
    }
    return t;
}
#define FILENAME "mahatma-gandhi-collected-works-volume-1.txt"

QNA_tool::QNA_tool() {
    // cout<<"parsing csv"<<endl;
    StandardText = parse_csv("unigram_freq.csv");
    // cout<<"parsed"<<endl;
}

QNA_tool::~QNA_tool() { delete StandardText; }

void heapify(vector<pair<double, Node *>> &t, int n, int i) {
    int k = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    // cout<<"left "<<left<<endl;
    // cout<<"right "<<right<<endl;
    if (left < n && t[left].first > t[k].first) {
        k = left;
    }
    if (right < n && t[right].first > t[k].first) {
        k = right;
    }
    if (k != i) {
        pair<double, Node *> temp = t[i];
        t[i] = t[k];
        t[k] = temp;
        heapify(t, n, k);
    }
}
void buildHeap(vector<pair<double, Node *>> &t, int n) {
    int start = (n / 2) - 1;
    for (int i = start; i > -1; i--) {
        heapify(t, n, i);
    }
}
void heapify_down(vector<pair<double, Node *>> &t) {
    int now = 0;
    int n = t.size();

    while (now < n) {
        // cout<<now<<" "<<flush;
        int left = 2 * now + 1;
        int right = 2 * now + 2;
        if (left < n && right < n) {
            if (t[left].first < t[right].first) {
                if (t[now].first < t[right].first) {
                    pair<double, Node *> temp = t[now];
                    t[now] = t[right];
                    t[right] = temp;
                }
                now = right;

            } else {
                if (t[now].first < t[left].first) {
                    pair<double, Node *> temp = t[now];
                    t[now] = t[left];
                    t[left] = temp;
                }
                now = left;
            }

        }

        else if (left < n) {
            if (t[now].first < t[left].first) {
                pair<double, Node *> temp = t[now];
                t[now] = t[left];
                t[left] = temp;
            }
            now = left;

        }

        else {
            return;
        }
    }
}
Node *get_top(vector<pair<double, Node *>> &t) {
    Node *top = t[0].second;

    t[0] = t[t.size() - 1];
    t.pop_back();

    heapify_down(t);

    return top;
}
Node *k_top(vector<pair<double, Node *>> &t, int k) {
    buildHeap(t, t.size());
    if (k > t.size()) {
        k = t.size();
    }

    Node *root = get_top(t);
    root->left = nullptr;
    Node *now = root;
    for (int i = 1; i < k; i++) {
        now->right = get_top(t);
        now->right->left = now;
        now = now->right;
    }
    now->right = nullptr;
    return root;
}

Node *QNA_tool::get_top_k_para(string question, int k) {
    // separating words in question
    vector<string> question_words;
    string word = "";
    for (int i = 0; i < question.size(); i++) {
        if (is_separator(question[i])) {
            if (word != "") {
                word = to_lower(word);  // converting to lower case
                question_words.push_back(word);
            }
            word = "";
        } else {
            word += question[i];
        }
    }
    if (word != "") {
        word = to_lower(word);
        question_words.push_back(word);
    }

    // making a trie of question words with frequency equal to thier frequency
    // in csv and corpus for faster score computation
    Trie *question_scores_csv = new Trie();
    Trie *question_scores_corpus = new Trie();

    for (string s : question_words) {
        question_scores_csv->set_freq(s, StandardText->get_freq(s));
        question_scores_corpus->set_freq(s, corpus->get_freq(s));
    }

    vector<pair<double, Node *>> ParaScores;

    for (long long book_num = 0; book_num < ParaTrie.size(); book_num++) {
        for (long long page = 0; page < ParaTrie[book_num].size(); page++) {
            for (long long para = 0; para < ParaTrie[book_num][page].size();
                 para++) {
                double score = 0;
                Trie *t = ParaTrie[book_num][page][para];
                for (string s : question_words) {
                    double numerator = question_scores_corpus->get_freq(s) + 1;
                    double denominator = question_scores_csv->get_freq(s) + 1;
                    double f = t->get_freq(s);
                    score += f * (numerator / denominator);
                }
                Node *n = new Node();
                n->book_code = book_num + 1;
                n->page = page;
                n->paragraph = para;
                pair<double, Node *> p = {score, n};
                ParaScores.push_back(p);
            }
        }
    }
    // cout << "PRINTING VECTOR:" << endl;
    // for (auto i : ParaScores) {
    //     cout << i.first << " "
    //          << " " << i.second->book_code << " " << i.second->page << " "
    //          << i.second->paragraph << endl;
    // }
    // cout << "------------" << endl;
    Node *head_of_k_maximum =
        new Node();  // a function that will give head of k maximum Nodes
    head_of_k_maximum = k_top(ParaScores, k);
    return head_of_k_maximum;
}

void QNA_tool::query(string question, string filename) {
    // Implement your function here
    //  std::cout << "Q: " << question << std::endl;
    // std::cout << "A: Studying COL106 :)" << std::endl;
    return;
}

void get_paragraph(int book_code, int page, int paragraph, int sentence_no,
                   string &res) {
    std::ifstream inputFile(FILENAME);

    std::string tuple;
    std::string sentence;

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the input file." << std::endl;
        exit(1);
    }

    while (std::getline(inputFile, tuple, ')') &&
           std::getline(inputFile, sentence)) {
        // Get a line in the sentence
        tuple += ')';

        int metadata[4];
        std::istringstream iss(tuple);

        // Temporary variables for parsing
        std::string token;

        // Ignore the first character (the opening parenthesis)
        iss.ignore(1);

        // Parse and convert the elements to integers
        int idx = 0;
        while (std::getline(iss, token, ',')) {
            // Trim leading and trailing white spaces
            size_t start = token.find_first_not_of(" ");
            size_t end = token.find_last_not_of(" ");
            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
            }

            // Check if the element is a number or a string
            if (token[0] == '\'') {
                // Remove the single quotes and convert to integer
                int num = std::stoi(token.substr(1, token.length() - 2));
                metadata[idx] = num;
            } else {
                // Convert the element to integer
                int num = std::stoi(token);
                metadata[idx] = num;
            }
            idx++;
        }

        if ((metadata[0] == book_code) && (metadata[1] == page) &&
            (metadata[2] == paragraph) && (metadata[3] == sentence_no)) {
            res = sentence;
            return;
        }

        res = "$I$N$V$A$L$I$D$";
        return;
    }
}
void QNA_tool::query_llm(string filename, Node *root, int k, string API_KEY,
                         string question) {
    // first write the k paragraphs into different files

    Node *traverse = root;
    int num_paragraph = 0;

    while (num_paragraph < k) {
        assert(traverse != nullptr);
        string p_file = "paragraph_";
        p_file += to_string(num_paragraph);
        p_file += ".txt";
        // delete the file if it exists
        remove(p_file.c_str());
        ofstream outfile(p_file);
        string paragraph = get_paragraph(traverse->book_code, traverse->page,
                                         traverse->paragraph);
        assert(paragraph != "$I$N$V$A$L$I$D$");
        outfile << paragraph;
        outfile.close();
        traverse = traverse->right;
        num_paragraph++;
    }

    // write the query to query.txt
    ofstream outfile("query.txt");
    outfile << "These are the excerpts from Mahatma Gandhi's books.\nOn the "
               "basis of this, ";
    outfile << question;
    // You can add anything here - show all your creativity and skills of using
    // ChatGPT
    outfile.close();

    // you do not need to necessarily provide k paragraphs - can configure
    // yourself

    // python3 <filename> API_KEY num_paragraphs query.txt
    string command = "python3 ";
    command += filename;
    command += " ";
    command += API_KEY;
    command += " ";
    command += to_string(k);
    command += " ";
    command += "query.txt";

    system(command.c_str());
    return;
}

void QNA_tool::insert_sentence(int book_code, int page, int paragraph,
                               int sentence_no, string sentence) {
    // checking if trie already exists or not

    while (book_code > ParaTrie.size()) {
        vector<vector<Trie *>> n;
        ParaTrie.push_back(n);
    }
    while (page + 1 > ParaTrie[book_code - 1].size()) {
        vector<Trie *> n;
        ParaTrie[book_code - 1].push_back(n);
    }
    while (paragraph + 1 > ParaTrie[book_code - 1][page].size()) {
        Trie *n = new Trie();
        ParaTrie[book_code - 1][page].push_back(n);
    }

    // inserting in both corpus and para tries

    string word = "";

    for (int i = 0; i < sentence.size(); i++) {
        if (is_separator(sentence[i])) {
            if (word != "") {
                word = to_lower(word);  // word to lower case
                corpus->increase_freq(word);

                ParaTrie[book_code - 1][page][paragraph]->increase_freq(word);
            }
            word = "";
        } else {
            word += sentence[i];
        }
    }

    if (word != "") {
        word = to_lower(word);
        corpus->increase_freq(word);
        ParaTrie[book_code - 1][page][paragraph]->increase_freq(word);
    }
}
std::string QNA_tool::get_paragraph(int book_code, int page, int paragraph) {
    cout << "Book_code: " << book_code << " Page: " << page
         << " Paragraph: " << paragraph << endl;

    std::string filename = "mahatma-gandhi-collected-works-volume-";
    filename += to_string(book_code);
    filename += ".txt";

    std::ifstream inputFile(filename);

    std::string tuple;
    std::string sentence;

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the input file " << filename << "."
                  << std::endl;
        exit(1);
    }

    std::string res = "";

    while (std::getline(inputFile, tuple, ')') &&
           std::getline(inputFile, sentence)) {
        // Get a line in the sentence
        tuple += ')';

        int metadata[5];
        std::istringstream iss(tuple);

        // Temporary variables for parsing
        std::string token;

        // Ignore the first character (the opening parenthesis)
        iss.ignore(1);

        // Parse and convert the elements to integers
        int idx = 0;
        while (std::getline(iss, token, ',')) {
            // Trim leading and trailing white spaces
            size_t start = token.find_first_not_of(" ");
            size_t end = token.find_last_not_of(" ");
            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
            }

            // Check if the element is a number or a string
            if (token[0] == '\'') {
                // Remove the single quotes and convert to integer
                int num = std::stoi(token.substr(1, token.length() - 2));
                metadata[idx] = num;
            } else {
                // Convert the element to integer
                int num = std::stoi(token);
                metadata[idx] = num;
            }
            idx++;
        }

        if ((metadata[0] == book_code) && (metadata[1] == page) &&
            (metadata[2] == paragraph)) {
            res += sentence;
        }
    }

    inputFile.close();
    return res;
}

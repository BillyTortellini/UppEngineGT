#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <list>
#include <string>

// Define datatypes
typedef uint8_t byte;
using std::vector;
using std::list;
using std::string;

void dealloc_file(char* data) {
    if (data != NULL) {
        delete[] data;
    }
}

// Allocates memory for the whole file, then reads the file into the specified memory area
char* read_whole_file(char* filepath) 
{
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        printf("File %s could not be openend!\n", filepath);
        return nullptr;
    }

    // Get File size
    fseek(file, 0, SEEK_END); 
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET); // Put cursor back to start of file

    // Alloc memory for file data
    char *data = new char[fileSize+1];
    
    // Read
    int readSize = fread(data, 1, fileSize, file); 
    if (readSize != fileSize) {
        printf("fread failed, it returned %d size instead of %d fileSize\n",
                readSize, fileSize);
        dealloc_file(data);
        fclose(file);
        return nullptr;
    }

    // Add null terminator
    data[fileSize] = '\0';

    return data;
}

vector<char*> init_keywords() 
{
    vector<char*> keywords;

    keywords.push_back("int");
    keywords.push_back("float");
    keywords.push_back("double");
    keywords.push_back("(");
    keywords.push_back(")");
    keywords.push_back("{");
    keywords.push_back("}");

    return keywords;
}

bool is_in_character_set(char c, char* set) {
    int len = strlen(set);
    for (int i = 0; i < len; i++) {
        if (c == set[i]) {
            return true;
        }
    }

    return false;
}

int find_next_characters(char* str, int pos, char* charsToFind) {
    int newPos = pos;
    while (str[newPos] != '\0') {
        if (is_in_character_set(str[newPos], charsToFind)) {
            return newPos;
        }
        newPos++;
    }
    return -1;
}

int skip_characters(char* str, int pos, char* charsToSkip) 
{
    int newPos = pos;
    int len = strlen(str);
    while (newPos < len && is_in_character_set(str[newPos], charsToSkip)) {
        newPos++;
    }
    return newPos;
}

bool starts_with(char* str, char* start) 
{
    int start_length = strlen(start);
    int str_length = strlen(str);
    if (str_length < start_length) return false;

    for (int i = 0; i < start_length; i++) {
        if (str[i] != start[i]) {
            return false;
        }
    }
    return true;
}

void get_line_and_char_number(char* str, int pos, int* lineNum, int* charNum) 
{
    *lineNum = 0;
    *charNum = 0;
    for (int i = 0; i < pos; i++) {
        if (str[i] == '\n') {
            *charNum = 0;
            *lineNum = *lineNum + 1;
        }
        else *charNum = *charNum + 1;
    }
}

bool tokenize_code(std::vector<int>* tokens, char* code, vector<char*> keywords)
{
    char* skippable_chars = " \n\r\t";
    int pos = skip_characters(code, 0, skippable_chars);
    int code_len = strlen(code);
    while (pos < code_len)
    {
        // Check if current word is in keyword list
        int token_index = -1;
        for (int i = 0; i < keywords.size(); i++) {
            if (starts_with(&code[pos], keywords[i])) {
                token_index = i;
                break;
            }
        }

        // If no token was found, print error msg
        if (token_index == -1) {
            int until = find_next_characters(code, pos, " \n\r\t;{}()");
            char buffer[1024];
            strncpy(buffer, &code[pos], until - pos);
            buffer[until-pos] = '\0';
            int lineNum, charNum;
            get_line_and_char_number(code, pos, &lineNum, &charNum);
            printf("Unrecognised token: %s at %d.%d\n", buffer, lineNum, charNum);
            return false;
        }
        else {
            tokens->push_back(token_index);
            // Move forwards the token_lengths amount
            int len = strlen(keywords[token_index]);
            pos += len;
        } 
        
        // Skip unneccessary tokens
        pos = skip_characters(code, pos, skippable_chars);
    }

    return true;
}

bool tokenize_file(vector<int>* tokens, char* filepath, vector<char*> keywords) 
{
    char* code = read_whole_file(filepath);
    if (code == nullptr) {
        printf("read_whole_file failed!\n");
        return false;
    }

    bool success = tokenize_code(tokens, code, keywords);
    dealloc_file(code);
    return success;
}

void print_tokens(vector<int>* tokens, vector<char*>* keywords) 
{
    printf("Printing token numbers:\n");
    for (int i = 0; i < tokens->size(); i++) {
        printf("%d ", (*tokens)[i]);
    }

    printf("\nPrinting tokens:\n");
    for (int i = 0; i < tokens->size(); i++) {
        printf("%s ", (*keywords)[(*tokens)[i]]);
    }
    printf("\n");        
}

void test_file(char* filepath) 
{
    printf("File: %s\n---------------------\n", filepath);

    vector<char*> keywords = init_keywords();
    vector<int> tokens;
    if (!tokenize_file(&tokens, filepath, keywords)) {
        printf("\n");
        system("pause");
        return;
    }
    print_tokens(&tokens, &keywords);
    printf("\n");
}


// ------------------------
// ------------------------
// ----- REGEX PARSER -----
// ------------------------
// ------------------------

/* What should my regex do?
 *  - Check if match
 * 
 * Possible strings in the regex
 * * 0 or multiple characters
 * . 1 or multiple
 * [abc] a or b or c
 * (a) something
 * (ab)*
 *
 * Goals: beeing able to do stuff like
 * "int[ \n]*[chars].[ \n];"
 *
 * Idee:
 *  Eventuell mehr in code form mit graph:
 *  Regex regex;
 *  Regex_single_char item = { 'i' }
 *  regex_set_start(item);
 */

// Epsilon is the leerwort
#define EPSILON 0
#define TRANSITION_TRAP -1
#define INVALID_SYMBOL -1
struct Automata
{
    int startingState;
    int maxStateCount;
    int stateCount;
    char** stateNames;
    int alphabetSize;
    char* alphabet;
    int** transitions;
    bool* endStates;
};

void print(Automata* a) 
{
    printf("Automata STATE:\n\tstartingState=%d\tstateCount=%d\t alphabetSize=%d\n", 
            a->startingState, a->stateCount, a->alphabetSize);
    printf("\tAlphabet: %s\n", a->alphabet);
    printf("\tEndStates: ");
    for (int i = 0; i < a->stateCount; i++) {
        if (a->endStates[i]) {
            printf("%s, ", a->stateNames[i]);
        }
    }
    printf("\n");
    printf("States:\n");
    for (int i = 0; i < a->stateCount; i++) {
        printf("\t%s\n", a->stateNames[i]);
        for (int j = 0; j < a->alphabetSize; j++) {
            if (a->transitions[i][j] != TRANSITION_TRAP) {
                printf("\t\t Transition %c => %s\n", a->alphabet[j], a->stateNames[a->transitions[i][j]]);
            }
        }
    }
}

void init(Automata* a, int maxStates, char* alphabet)
{
    a->startingState = 0;
    a->maxStateCount = maxStates;
    a->stateCount = 0; 
    a->stateNames = new char*[maxStates];
    memset(a->stateNames, 0, sizeof(char*) * maxStates);
    a->alphabetSize = strlen(alphabet);
    a->alphabet = new char[a->alphabetSize+1];
    strcpy(a->alphabet, alphabet);
    a->transitions = new int*[maxStates];
    for (int i = 0; i < maxStates; i++) {
        a->transitions[i] = new int[a->alphabetSize];
        for (int j = 0; j < a->alphabetSize; j++) {
            a->transitions[i][j] = TRANSITION_TRAP;
        }
    }
    a->endStates = new bool[maxStates];
    memset(a->endStates, 0, sizeof(bool) * maxStates);
}

void shutdown(Automata* a) 
{
    for (int i = 0; i < a->stateCount; i++) {
        delete a->stateNames[i];
    }
    delete[] a->stateNames;
    delete[] a->alphabet;
    for (int i = 0; i < a->maxStateCount; i++) {
        delete[] a->transitions[i];
    }
    delete[] a->transitions;
    delete[] a->endStates;
}

int state_name_index(Automata* a, char* stateName) 
{
    for (int i = 0; i < a->stateCount; i++) {
        if (strcmp(a->stateNames[i], stateName) == 0) {
            return i;
        }
    }
    return TRANSITION_TRAP;
}

void add_state(Automata* a, char* stateName) 
{
    if (state_name_index(a, stateName) != TRANSITION_TRAP) {
        printf("add_state failed: stateName %s already exists\n", stateName);
        return;
    }

    int stateIndex = a->stateCount;
    ++(a->stateCount);
    if (a->stateCount > a->maxStateCount) {
        a->stateCount = a->maxStateCount;
        printf("Max state count %d reached, could not add state %s\n", a->maxStateCount, stateName);
        return;
    }
    int len = strlen(stateName);
    a->stateNames[stateIndex] = new char[len+1];
    strcpy(a->stateNames[stateIndex], stateName);
}

int symbol_index(Automata* a, char symbol) 
{
    for (int i = 0; i < a->alphabetSize; i++) {
        if (symbol == a->alphabet[i]) {
            return i;
        }
    }
    return INVALID_SYMBOL;
}

void add_transition(Automata* a, int from, int to, char symbol) 
{
    // Parameter checks
    if (from < 0 || from >= a->stateCount) {
        printf("Add_transition failed: From is not in valid range [0, %d], was %d\n", a->stateCount, from);
        return;
    }
    if (to < 0 || to >= a->stateCount) {
        printf("Add_transition failed: To is not in valid range [0, %d], was %d\n", a->stateCount, to);
        return;
    }

    int symbolIndex = symbol_index(a, symbol);
    if (symbolIndex == INVALID_SYMBOL) {
        printf("Add_transition failed: Symbol %c not found in alphabet %s\n", symbol, a->alphabet);
        return;
    }
    a->transitions[from][symbolIndex] = to;
}

void add_transition(Automata* a, int from, int to, char* symbols) 
{
    int len = strlen(symbols);
    for (int i = 0; i < len; i++) {
        add_transition(a, from, to, symbols[i]);
    }
}

void add_transition(Automata* a, char* from, char* to, char symbol) 
{
    int fromState = state_name_index(a, from);
    int toState = state_name_index(a, to);
    add_transition(a, fromState, toState, symbol);
}

void add_transition(Automata* a, char* from, char* to, char* symbols) 
{
    int fromState = state_name_index(a, from);
    int toState = state_name_index(a, to);
    add_transition(a, fromState, toState, symbols);
}

void remove_transition(Automata* a, int from, char symbol) 
{
    add_transition(a, from, TRANSITION_TRAP, symbol);
}

void remove_transition(Automata* a, int from, char* symbols) 
{
    add_transition(a, from, TRANSITION_TRAP, symbols);
}

void set_start(Automata* a, int startState) {
    a->startingState = startState;
}

void flag_end(Automata* a, int stateIndex) {
    if (stateIndex < 0 || stateIndex >= a->stateCount) {
        printf("flag_endState failed: stateIndex %d not valid\n", stateIndex);
        return;
    }
    a->endStates[stateIndex] = true;
}

void flag_end(Automata* a, char* stateName) {
    int stateIndex = state_name_index(a, stateName);
    flag_end(a, stateIndex); 
}

bool accepts(Automata* a, char* word) 
{
    // Check if word only contains automata characters
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        if (symbol_index(a, word[i]) == INVALID_SYMBOL) {
            printf("Accepts failed: input character %d:'%c' is not in alphabet %s\n", i, word[i], a->alphabet);
            return false;
        }
    }
    // Go through word step by step and go into next state
    int currentState = a->startingState;
    for (int i = 0; i < len; i++) {
        currentState = a->transitions[currentState][symbol_index(a, word[i])];
        if (currentState == TRANSITION_TRAP) {
            return false;
        }
    }

    // Check if last state is an endState
    return a->endStates[currentState];
}

#define set vector<int>
bool contains(set* s, int x) 
{
    for (int i = 0; i < s->size(); i++) {
        if ((*s)[i] == x) {
            return true;
        }
    }
    return false;
}

bool add(set* s, int x) {
    bool changed = false;
    if (!contains(s, x)) {
        s->push_back(x);
        changed = true;
    }
    return changed;
}

bool add(set* dest, set* src) {
    bool changed = false;
    for (int i = 0; i < (*src).size(); i++)
    {
        changed = changed || add(dest, (*src)[i]);
    }
    return changed;
}

bool equals(set* a, set* b)
{
    if (a->size() != b->size()) {
        return false;
    }
    for (int i = 0; i < a->size(); i++) {
        if (!contains(b, (*a)[i])) {
            return false;
        }
    }
    return true;
}

#define MAX_SYMBOLS (256)
struct State
{
    string name;
    set transitions[MAX_SYMBOLS];
    bool endState;
};

struct NEA
{
    int start;
    string alphabet;
    int symbolIndexLookup[MAX_SYMBOLS];
    vector<State> states;
};

void init(NEA* a, const string& alphabet)
{
    a->start = 0;
    a->alphabet = std::to_string(' ') + alphabet;
    a->alphabet[0] = '\0';
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        a->symbolIndexLookup[i] = INVALID_SYMBOL;
    }
    for (int i = 0; i < a->alphabet.size(); i++) {
        a->symbolIndexLookup[a->alphabet[i]] = i;
    }
}

int state_count(NEA* a) {
    return (int) a->states.size();
}

set* get_symbol_transitions(NEA* a, int state, int symbol) {
    return &(a->states[state].transitions[symbol]);
}

void shutdown(NEA* a) {
}

void add_state(NEA* a, const string& name) 
{
    State state;
    state.name = name;
    state.endState = false;
    a->states.push_back(state);
}

int get_state_index(NEA* a, const string& name)
{
    for (int i = 0; i < state_count(a); i++) {
        if (a->states[i].name == name) {
            return i;
        }
    }

    printf("Get_state_index failed: name '%s' was not found\n", name.c_str());
    return -1;
}

bool check_valid_index(NEA* a, int index) {
    return index >= 0 && index < state_count(a);
}

void add_transition(NEA* a, int from, int to, char symbol)
{
    if (!check_valid_index(a, from) || !check_valid_index(a, to)) {
        printf("add_transition failed: to '%d' or from '%d' were out of range %d\n", to, from, (int)state_count(a));
        return;
    }
    int symbolIndex = a->symbolIndexLookup[symbol];
    if (symbolIndex == INVALID_SYMBOL) {
        printf("add_transition failed: invalid symbol '%c'\n", symbol);
        return;
    }

    add(get_symbol_transitions(a, from, symbolIndex), to);
}

void add_transition(NEA* a, char* from, char* to, char symbol)
{
    int fromIndex = get_state_index(a, from);
    int toIndex = get_state_index(a, to);
    add_transition(a, fromIndex, toIndex, symbol);
}

void flag_end(NEA* a, int state) {
    if (check_valid_index(a, state)) {
        a->states[state].endState = true;   
    }
    else {
        printf("Flag_end failed: state index not valid\n");
    }
};

void print_set(NEA* a, set* s) 
{
    printf("{");
    for (int i = 0; i < s->size(); i++) {
        printf("%s, ", a->states[(*s)[i]].name.c_str());
    }
    printf("}");
}

void flag_end(NEA* a, char* name) {
    int state = get_state_index(a, name);
    flag_end(a, state);
}

bool add_transitions_to_set(NEA* a, set* nextState, set* currState, char symbol)
{
    // Get symbol from input
    int index = a->symbolIndexLookup[symbol];
    if (symbol == INVALID_SYMBOL) {
        printf("add_transitions_to_set warning: symbol '%c' is not in alphabet!\n", symbol);
        return false;
    }
    
    // Add all transitions to set
    bool changed = false;
    for (int i = 0; i < currState->size(); i++) 
    {
        int state = (*currState)[i];
        set* transitions = get_symbol_transitions(a, state, index);
        for (int i = 0; i < transitions->size(); i++) {
            changed = changed || add(nextState, (*transitions)[i]);
        }
    }

    return changed;
}

bool accepts(NEA* a, const string& input)
{
    // TODO:
    // Because epsilon transitions may happen without any input, we have to loop over epsilon 
    // transitions after each input change state until no changes are added to the state variable

    // Sets for swapping
    set s1, s2, s3;
    set* currState = &s1;
    set* nextState = &s2;
    set* reachable = &s3;


    // Setup start
    currState->push_back(a->start);

    /*
     * set added, curr, next;
     * transitions(added, currState);
     * add(currState, added)
     * while (transitions(nextState, added))
     *      if !add(currState, nextState);
     *          break; 
     */
    // Do epsilon transitions
    bool changed = false;
    add_transitions_to_set(a, nextState, currState, EPSILON);
    changed = add(currState, nextState);

    while (changed)
    {
        add_transitions_to_set(a, reachable, nextState, EPSILON);
        changed = add(currState, reachable);

        set* tmp = reachable;
        reachable = nextState;
        nextState = tmp;
        reachable->clear();
    }

    for (int i = 0; i < input.size(); i++)
    {
        // Check for early end
        if (currState->size() == 0) {
            return false;
        }

        // Do epsilon transitions
        changed = false;
        add_transitions_to_set(a, nextState, currState, EPSILON);
        changed = add(currState, nextState);

        while (changed)
        {
            add_transitions_to_set(a, reachable, nextState, EPSILON);
            changed = add(currState, reachable);

            set* tmp = reachable;
            reachable = nextState;
            nextState = tmp;
            reachable->clear();
        }

        // Add transitions
        add_transitions_to_set(a, nextState, currState, input[i]);

        // Swap state with swap
        set* tmp = currState;
        currState = nextState;
        nextState = tmp;
        nextState->clear();

        printf("After symbol %d '%c' states: ", i, input[i]);
        print_set(a, currState);
        printf("\n");
    }

    // Do epsilon transitions
    changed = false;
    add_transitions_to_set(a, nextState, currState, EPSILON);
    changed = add(currState, nextState);

    while (changed)
    {
        add_transitions_to_set(a, reachable, nextState, EPSILON);
        changed = add(currState, reachable);

        set* tmp = reachable;
        reachable = nextState;
        nextState = tmp;
        reachable->clear();
    }

    // Check if an end state is available
    for (int i = 0; i < currState->size(); i++) {
        if (a->states[(*currState)[i]].endState) {
            return true;
        }
    }

    return false;
}

void print(NEA* a)
{
    printf("NEA State: \n\tstart=%d\tstateCount=%d\t alphabetLength=%d\n", 
            a->start, state_count(a), (int)a->alphabet.size());
    printf("\tAlphabet: %s\n", a->alphabet.c_str());
    printf("\tEndStates: ");
    for (int i = 0; i < state_count(a); i++) {
        State* s = &(a->states[i]);
        if (s->endState) {
            printf("%s, ", s->name.c_str());
        }
    }
    printf("\n");
    printf("States:\n");
    for (int i = 0; i < state_count(a); i++) 
    {
        State* s = &(a->states[i]);
        printf("\t%s\n", s->name.c_str());
        for (int j = 0; j < a->alphabet.size(); j++) 
        {
            set* transitions = get_symbol_transitions(a, i, j);
            for (int k = 0; k < transitions->size(); k++) {
                if (a->alphabet[j] == EPSILON) {
                    printf("\t\t 'Epsilon' => %s\n", a->states[(*transitions)[k]].name.c_str());
                }
                else {
                    printf("\t\t '%c' => %s\n", a->alphabet[j], a->states[(*transitions)[k]].name.c_str());
                }
            }
        }
    }
}


int main(int argc, char** argv) 
{
    //test_file("examples/datatypes.upp");
    //test_file("examples/error_type.upp");

    NEA a;
    init(&a, "abcdef");
    add_state(&a, "A");
    add_state(&a, "B");
    add_state(&a, "C");
    add_state(&a, "D");
    add_state(&a, "E");
    add_transition(&a, "A", "B", 'a');
    add_transition(&a, "B", "C", EPSILON);
    add_transition(&a, "C", "D", EPSILON);
    add_transition(&a, "D", "E", EPSILON);
    flag_end(&a, "E");
    a.start = 0;
    print(&a);

    char* str1 = "ae";
    char* str2 = "";
    char* str3 = "a";
    char* str4 = "dddees";

    printf("Accepts \"%s\": %s\n", str1, accepts(&a, str1) ? "TRUE" : "FALSE");
    printf("Accepts \"%s\": %s\n", str2, accepts(&a, str2) ? "TRUE" : "FALSE");
    printf("Accepts \"%s\": %s\n", str3, accepts(&a, str3) ? "TRUE" : "FALSE");
    printf("Accepts \"%s\": %s\n", str4, accepts(&a, str4) ? "TRUE" : "FALSE");

    //determinize(&a);

    //Automata a;
    //init(&a, 20, "abcdef");
    //add_state(&a, "A");
    //add_state(&a, "B");
    //add_state(&a, "C");
    //add_state(&a, "D");
    //add_state(&a, "D");
    //add_transition(&a, "A", "B", "abc");
    //add_transition(&a, "B", "B", "d");
    //add_transition(&a, "B", "C", "ce");
    //flag_end(&a, "C");

    //print(&a);
    //printf("\n");

    //shutdown(&a);

    //system("pause");

    return 0;
}

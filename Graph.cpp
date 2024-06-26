#include <iostream>
#include <fstream>
#include <cstring>
#include <ncurses.h>
#include <vector>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <stack>
#include <sstream>
#include <regex>
#include "AVLTree.h"

using namespace std;

int e_id = 1; // Global event ID counter

class EventGraph {
private:
    vector<Event> events;
    vector<vector<int>> dependencies;

public:
    void addEvent(const Event& event) {
        events.push_back(event);
        dependencies.push_back({});
    }

    bool hasCycleUtil(int v, unordered_map<int, bool>& visited, unordered_map<int, bool>& recStack) const {
        if (!visited[v]) {
            visited[v] = true;
            recStack[v] = true;

            for (int dep : findEventById(v).dependencies) {
                if (!visited[dep] && hasCycleUtil(dep, visited, recStack)) {
                    return true;
                } else if (recStack[dep]) {
                    return true;
                }
            }
        }
        recStack[v] = false;
        return false;
    }

    bool hasCycle() const {
        unordered_map<int, bool> visited;
        unordered_map<int, bool> recStack;
        for (const auto& event : events) {
            if (hasCycleUtil(event.id, visited, recStack)) {
                return true;
            }
        }
        return false;
    }

    void addDependency(int fromEventId, int toEventId) {
        int fromIndex = -1, toIndex = -1;
        for (size_t i = 0; i < events.size(); ++i) {
            if (events[i].id == fromEventId) fromIndex = i;
            if (events[i].id == toEventId) toIndex = i;
        }
        if (fromIndex != -1 && toIndex != -1) {
            dependencies[fromIndex].push_back(toEventId);
            events[fromIndex].dependencies.insert(toEventId);
            if (hasCycle()) {
                dependencies[fromIndex].pop_back();
                events[fromIndex].dependencies.erase(toEventId);
                throw runtime_error("Adding this dependency creates a cycle");
            }
        }
    }

    void updateEventName(int id, const string& newName) {
        for (auto& event : events) {
            if (event.id == id) {
                event.name = newName;
                break;
            }
        }
    }

    void updateEventDate(int id, const string& newDate) {
        for (auto& event : events) {
            if (event.id == id) {
                event.date = newDate;
                break;
            }
        }
    }

    void updateEventStartTime(int id, const string& newStartTime) {
        for (auto& event : events) {
            if (event.id == id) {
                event.startTime = newStartTime;
                break;
            }
        }
    }

    void updateEventEndTime(int id, const string& newEndTime) {
        for (auto& event : events) {
            if (event.id == id) {
                event.endTime = newEndTime;
                break;
            }
        }
    }

    void deleteEvent(int id) {
        events.erase(remove_if(events.begin(), events.end(), [id](const Event& event) {
            return event.id == id;
        }), events.end());
    }
    
    Event findEventById(int id) const {
    for (const auto& event : events) {
        if (event.id == id) {
            return event;
        }
    }
    throw runtime_error("Event not found");
}

void topologicalSortUtil(int v, unordered_map<int, bool>& visited, stack<int>& Stack) {
    // Mark the current node as visited
    visited[v] = true;

    // Recur for all the vertices adjacent to this vertex
    for (int dep : findEventById(v).dependencies) {
        if (!visited[dep]) {
            topologicalSortUtil(dep, visited, Stack);
        }
    }

    // Push current vertex to stack which stores the result
    Stack.push(v);
}

// Topological sort function
vector<Event> topologicalSort() {
    stack<int> Stack;
    unordered_map<int, bool> visited;
    vector<Event> sortedEvents;

    // Mark all the vertices as not visited
    for (const auto& event : events) {
        visited[event.id] = false;
    }

    // Call the recursive helper function to store Topological Sort starting from all vertices one by one
    for (const auto& event : events) {
        if (!visited[event.id]) {
            topologicalSortUtil(event.id, visited, Stack);
        }
    }

    // Pop all vertices from stack and add to the sorted list
    while (!Stack.empty()) {
        sortedEvents.push_back(findEventById(Stack.top()));
        Stack.pop();
    }

    return sortedEvents;
}

    void exportGraph(const string& filename) const {
        ofstream outfile(filename);
        outfile << "digraph EventGraph {\n";
        outfile << "node [fontname=\"Arial\"];\n";

        for (const auto& event : events) {
            outfile << "\"" << event.name << "\" [label=\"" << event.name << "\\n" << event.date << "\\n" << event.startTime << "-" << event.endTime << "\"];\n";
        }

        outfile << "}\n";
    }

    void viewSchedule() const {
        clear();
        mvprintw(0, 0, "Event Schedule:");
        int row = 1;
        for (const auto& event : events) {
            mvprintw(row++, 0, "%d: %s (%s %s-%s)", event.id, event.name.c_str(), event.date.c_str(), event.startTime.c_str(), event.endTime.c_str());
        }
        refresh();
        mvprintw(row, 0, "Press any key to return to the main menu...");
        getch();
    }

    bool hasConflict(const Event& newEvent) const {
    for (const auto& event : events) {
        if (newEvent.date == event.date && 
            !(newEvent.endTime <= event.startTime || newEvent.startTime >= event.endTime)) {
            return true;
        }
    }
    return false;
}


    void loadEvents(const string& filename, AVLTree& avlTree) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        return;
    }

    events.clear();
    dependencies.clear();
    string line;
    int maxId = 0;

    // First pass to load all events
    while (getline(infile, line)) {
        stringstream ss(line);
        string token;
        Event event;

        getline(ss, token, ',');
        event.id = stoi(token);
        getline(ss, token, ',');
        event.name = token;
        getline(ss, token, ',');
        event.date = token;
        getline(ss, token, ',');
        event.startTime = token;
        getline(ss, token, ',');
        event.endTime = token;

        events.push_back(event);
        dependencies.push_back({}); // Initialize empty dependencies for each event

        avlTree.insert(event); // Insert event into AVL tree

        if (event.id > maxId) {
            maxId = event.id;
        }
    }

    e_id = maxId + 1; // Initialize e_id to one more than the highest ID

    // Second pass to add dependencies
    infile.clear();
    infile.seekg(0, ios::beg);

    while (getline(infile, line)) {
        stringstream ss(line);
        string token;
        getline(ss, token, ',');
        int eventId = stoi(token);

        // Skip event details
        for (int i = 0; i < 5; ++i) {
            getline(ss, token, ',');
        }

        while (getline(ss, token, ',')) {
            int depId = stoi(token);
            addDependency(eventId, depId); // Add dependency for the event
        }
    }
}


    void visualize_event_graph() const {
    clear();
    ofstream outfile("eventgraph.dot");
    if (!outfile) {
        cerr << "Error creating dot file" << endl;
        return;
    }
    outfile << "digraph EventGraph {\n";
    for (const auto& event : events) {
        outfile << event.id << " [label=\"" << event.name << "\\n" << event.date << "\\n" << event.startTime << "-" << event.endTime << "\"];\n";
    }
    for (size_t i = 0; i < events.size(); ++i) {
        for (int dep : dependencies[i]) {
            outfile << events[i].id << " -> " << dep << ";\n";
        }
    }
    outfile << "}\n";
    outfile.close();
    system("dot -Tpng eventgraph.dot -o eventgraph.png");
    system("xdg-open eventgraph.png");
    mvprintw(2, 0, "Event graph exported and visualized");
    mvprintw(4, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}


    void saveEvents(const string& filename) const {
    ofstream outfile(filename);
    for (const auto& event : events) {
        outfile << event.id << "," << event.name << "," << event.date << "," 
                << event.startTime << "," << event.endTime;
        for (int dep : event.dependencies) {
            outfile << "," << dep;
        }
        outfile << endl;
    }
}


};

// Initialize Ncurses
void initialize_ncurses() {
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_GREEN, COLOR_BLACK);
}

// Draw a box with given dimensions and color
void draw_box(int starty, int startx, int height, int width, int color_pair) {
    attron(COLOR_PAIR(color_pair));
    for (int i = startx; i < startx + width; ++i) {
        mvprintw(starty, i, "-");
        mvprintw(starty + height, i, "-");
    }
    for (int i = starty; i < starty + height; ++i) {
        mvprintw(i, startx, "|");
        mvprintw(i, startx + width, "|");
    }
    mvprintw(starty, startx, "+");
    mvprintw(starty + height, startx, "+");
    mvprintw(starty, startx + width, "+");
    mvprintw(starty + height, startx + width, "+");
    attroff(COLOR_PAIR(color_pair));
}

// Display Menu with enhanced UI
void display_menu() {
    clear();
    int starty = (LINES - 15) / 2;
    int startx = (COLS - 50) / 2;
    draw_box(starty, startx, 16, 40, 2);

    attron(COLOR_PAIR(3));
    mvprintw(starty + 1, startx + 13, "Event Scheduler");
    attroff(COLOR_PAIR(3));

    mvprintw(starty + 3, startx + 5, "1. Create Event");
    mvprintw(starty + 4, startx + 5, "2. Update Event");
    mvprintw(starty + 5, startx + 5, "3. Delete Event");
    mvprintw(starty + 6, startx + 5, "4. View Schedule");
    mvprintw(starty + 7, startx + 5, "5. Visualize Event Graph");
    mvprintw(starty + 8, startx + 5, "6. Visualize AVL Tree");
    mvprintw(starty + 9, startx + 5, "7. Add Dependency");
    mvprintw(starty + 10, startx + 5, "8. Topological Sort"); // New option for topological sort
    mvprintw(starty + 11, startx + 5, "9. Search Event");
    mvprintw(starty + 12, startx + 5, "10. Exit");
    mvprintw(starty + 14, startx + 5, "Enter your choice: "); // Adjusted for the new option

    refresh();
}

bool validate_date(const string& date) {
    regex date_pattern("^\\d{4}-\\d{2}-\\d{2}$");
    return regex_match(date, date_pattern);
}

bool validate_time(const string& time) {
    regex time_pattern("^\\d{2}:\\d{2}$");
    return regex_match(time, time_pattern);
}

void create_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    mvprintw(0, 0, "Enter event name: ");
    char name[100];
    getstr(name);

    string date;
    while (true) {
        mvprintw(1, 0, "Enter event date (YYYY-MM-DD): ");
        char date_cstr[20];
        getstr(date_cstr);
        date = string(date_cstr);
        if (validate_date(date)) break;
        mvprintw(2, 0, "Invalid date format. Please enter again.");
    }

    string startTime;
    while (true) {
        mvprintw(3, 0, "Enter event start time (HH:MM): ");
        char startTime_cstr[10];
        getstr(startTime_cstr);
        startTime = string(startTime_cstr);
        if (validate_time(startTime)) break;
        mvprintw(4, 0, "Invalid time format. Please enter again.");
    }

    string endTime;
    while (true) {
        mvprintw(5, 0, "Enter event end time (HH:MM): ");
        char endTime_cstr[10];
        getstr(endTime_cstr);
        endTime = string(endTime_cstr);
        if (validate_time(endTime)) break;
        mvprintw(6, 0, "Invalid time format. Please enter again.");
    }

    Event newEvent(e_id, name, date, startTime, endTime);

    if (graph.hasConflict(newEvent)) {
        mvprintw(8, 0, "Error: Event conflicts with existing events.");
        refresh();
        getch();
        return;
    }

    graph.addEvent(newEvent);
    avlTree.insert(newEvent);
    e_id++;

    mvprintw(8, 0, "Event created successfully.");
    mvprintw(10, 0, "Your Event-id is: %d", e_id - 1);
    mvprintw(12, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void update_event(EventGraph& graph) {
    clear();
    mvprintw(0, 0, "Enter event ID to update: ");
    int id;
    scanw("%d", &id);

    mvprintw(1, 0, "Enter new event name (leave empty to keep current): ");
    char name[100];
    getstr(name);

    string date;
    while (true) {
        mvprintw(2, 0, "Enter new event date (YYYY-MM-DD) (leave empty to keep current): ");
        char date_cstr[20];
        getstr(date_cstr);
        date = string(date_cstr);
        if (date.empty() || validate_date(date)) break;
        mvprintw(3, 0, "Invalid date format. Please enter again.");
    }

    string startTime;
    while (true) {
        mvprintw(4, 0, "Enter new event start time (HH:MM) (leave empty to keep current): ");
        char startTime_cstr[10];
        getstr(startTime_cstr);
        startTime = string(startTime_cstr);
        if (startTime.empty() || validate_time(startTime)) break;
        mvprintw(5, 0, "Invalid time format. Please enter again.");
    }

    string endTime;
    while (true) {
        mvprintw(6, 0, "Enter new event end time (HH:MM) (leave empty to keep current): ");
        char endTime_cstr[10];
        getstr(endTime_cstr);
        endTime = string(endTime_cstr);
        if (endTime.empty() || validate_time(endTime)) break;
        mvprintw(7, 0, "Invalid time format. Please enter again.");
    }

    if (strlen(name) > 0) {
        graph.updateEventName(id, name);
    }
    if (!date.empty()) {
        graph.updateEventDate(id, date);
    }
    if (!startTime.empty()) {
        graph.updateEventStartTime(id, startTime);
    }
    if (!endTime.empty()) {
        graph.updateEventEndTime(id, endTime);
    }

    mvprintw(9, 0, "Event updated successfully.");
    mvprintw(11, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void delete_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    mvprintw(0, 0, "Enter event ID to delete: ");
    int id;
    scanw("%d", &id);
    graph.deleteEvent(id);
    avlTree.remove(id);
    mvprintw(2, 0, "Event deleted successfully.");
    refresh();
    getch();
}

void add_dependency(EventGraph& graph) {
    clear();
    mvprintw(0, 0, "Enter the ID of the event to depend on: ");
    int fromEventId;
    scanw("%d", &fromEventId);

    mvprintw(1, 0, "Enter the ID of the dependent event: ");
    int toEventId;
    scanw("%d", &toEventId);

    graph.addDependency(fromEventId, toEventId);

    mvprintw(3, 0, "Dependency added successfully.");
    mvprintw(5, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

int main() {
    initialize_ncurses();

    EventGraph graph;
    AVLTree avlTree;

    string events_filename = "events.txt";
    graph.loadEvents(events_filename, avlTree); // Load events and insert into AVL tree

    int choice;
    while (true) {
        display_menu();
        scanw("%d", &choice);

        switch (choice) {
        case 1:
            create_event(graph, avlTree);
            graph.saveEvents(events_filename); // Save events after creating
            break;
        case 2:
            update_event(graph);
            graph.saveEvents(events_filename); // Save events after updating
            break;
        case 3:
            delete_event(graph, avlTree);
            graph.saveEvents(events_filename); // Save events after deleting
            break;
        case 4:
            graph.viewSchedule();
            break;
        case 5:
            graph.visualize_event_graph();
            break;
        case 6:
            avlTree.visualize(); // Implement this function in your AVLTree class
            break;
        case 7:
            try{
                add_dependency(graph);
            }catch (const runtime_error& e) {
                mvprintw(2, 0, "Error: %s", e.what());
            }
            mvprintw(LINES - 1, 0, "Press any key to return to the main menu...");
            getch();
            break;
        case 8: {
            clear();
            mvprintw(0, 0, "Events in topological order:");
            try {
                vector<Event> sortedEvents = graph.topologicalSort();
                int row = 1;
                for (const auto& event : sortedEvents) {
                    mvprintw(row++, 0, "Event-id: %d", event.id);
                }
            } catch (const runtime_error& e) {
                mvprintw(2, 0, "Error: %s", e.what());
            }
            refresh();
            mvprintw(5, 0, "Press any key to return to the main menu...");
            getch();
            break;
        }
        case 9: {
    clear();
    mvprintw(0, 0, "Enter the event-id: ");
    int t;
    scanw("%d", &t);
    try {
       Event event;
       event = graph.findEventById(t);
       mvprintw(2, 0, "Event-id: %d\nEvent name: %s\nDate:%s\nTiming: %s-%s", event.id, event.name.c_str(), event.date.c_str(), event.startTime.c_str(), event.endTime.c_str());
    } catch (const runtime_error& e) {
        mvprintw(3, 0, "Error: %s", e.what());
        mvprintw(5, 0, "Press any key to return to the main menu...");
        refresh();
        getch();
        break; // Added to exit the switch statement
    }
    mvprintw(6, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
    break; // Added to exit the switch statement
}

        case 10:
            endwin(); // End ncurses mode
            return 0;
        default:
            break;
        }
    }

    return 0;
}

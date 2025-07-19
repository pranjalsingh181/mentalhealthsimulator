#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

// Constants for file operations
const string USER_DATA_FILE = "user_data.txt";
const string MOOD_HISTORY_FILE = "mood_history_";

// User class with file persistence
class User {
private:
    string name;
    vector<string> moodHistory;

    void loadMoodHistory() {
        string filename = MOOD_HISTORY_FILE + name + ".txt";
        ifstream inFile(filename);
        if (inFile.is_open()) {
            string mood;
            while (getline(inFile, mood)) {
                moodHistory.push_back(mood);
            }
            inFile.close();
        }
    }

public:
    User(string name) : name(name) {
        loadMoodHistory();
    }

    string getName() const {
        return name;
    }

    void addMoodHistory(string mood) {
        moodHistory.push_back(mood);
        // Save to file immediately
        ofstream outFile(MOOD_HISTORY_FILE + name + ".txt", ios::app);
        if (outFile.is_open()) {
            outFile << mood << endl;
            outFile.close();
        }
    }

    vector<string> getMoodHistory() const {
        return moodHistory;
    }

    string getMoodTrend() const {
        if (moodHistory.empty()) return "No trend data available";

        map<string, int> moodCounts;
        for (const auto& mood : moodHistory) {
            moodCounts[mood]++;
        }

        auto maxMood = max_element(moodCounts.begin(), moodCounts.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second < b.second;
            });

        return maxMood->first + " (appears " + to_string(maxMood->second) + " times)";
    }

    vector<string> getRecentMoods(int days = 3) const {
        int count = min(days, static_cast<int>(moodHistory.size()));
        vector<string> recentMoods;
        for (int i = moodHistory.size() - count; i < moodHistory.size(); i++) {
            recentMoods.push_back(moodHistory[i]);
        }
        return recentMoods;
    }
};

// Enhanced CodeProvider with mood-specific quotes
class CodeProvider {
private:
    map<string, vector<string>> moodQuotes = {
        {"Happy", {
            "Joy is the simplest form of gratitude. - Karl Barth",
            "Happiness is not something ready made. It comes from your own actions. - Dalai Lama",
            "The happiest people don't have the best of everything, they make the best of everything."
        }},
        {"Sad", {
            "This feeling will pass. The fear is real but the danger is not.",
            "You're allowed to feel messed up and inside out. It doesn't mean you're defective - it means you're human.",
            "Tears water our growth. - William Shakespeare"
        }},
        {"Anxious", {
            "You don't have to control your thoughts. You just have to stop letting them control you.",
            "Anxiety is a thin stream of fear trickling through the mind. If encouraged, it cuts a channel into which all other thoughts are drained.",
            "Breathe. It's just a bad day, not a bad life."
        }},
        {"Stressed", {
            "You can't stop the waves, but you can learn to surf. - Jon Kabat-Zinn",
            "It's not the load that breaks you down, it's the way you carry it. - Lou Holtz",
            "Stress is caused by being 'here' but wanting to be 'there'."
        }},
        {"Angry", {
            "For every minute you remain angry, you give up sixty seconds of peace of mind. - Ralph Waldo Emerson",
            "Anger is an acid that can do more harm to the vessel in which it is stored than to anything on which it is poured.",
            "Speak when you are angry and you will make the best speech you will ever regret."
        }}
    };

    vector<string> genericQuotes = {
        "Believe you can and you're halfway there. - Theodore Roosevelt",
        "You are never too old to set another goal or to dream a new dream. - C.S. Lewis",
        "Progress, not perfection.",
        "You've survived 100% of your bad days so far."
    };

public:
    string getRandomQuote(const string& mood = "") {
        srand(time(0));
        
        if (!mood.empty() && moodQuotes.find(mood) != moodQuotes.end()) {
            const auto& quotes = moodQuotes[mood];
            return quotes[rand() % quotes.size()];
        }
        
        return genericQuotes[rand() % genericQuotes.size()];
    }
};

// Enhanced MoodTracker with ImGui
class MoodTracker {
private:
    vector<string> moodOptions = {
        "Happy", "Sad", "Anxious", "Stressed", "Angry",
        "Excited", "Tired", "Peaceful", "Confused", "Hopeful"
    };
    int selectedMood = -1;
    string currentMood = "";

public:
    string askMood(User* user) {
        if (selectedMood != -1) {
            currentMood = moodOptions[selectedMood];
            selectedMood = -1; // Reset selection
            return currentMood;
        }
        return "";
    }

    void renderMoodSelection(User* user) {
        ImGui::Text("Hi %s, how are you feeling today?", user->getName().c_str());
        ImGui::Text("Please choose from the following options:");
        
        for (int i = 0; i < moodOptions.size(); i++) {
            if (ImGui::RadioButton(moodOptions[i].c_str(), selectedMood == i)) {
                selectedMood = i;
            }
        }
    }

    bool isMoodSelected() const {
        return selectedMood != -1;
    }
};

// Enhanced CalmingActivityProvider with ImGui
class CalmingActivityProvider {
private:
    bool showBreathingExercise = false;
    bool showGroundingExercise = false;
    bool showMuscleRelaxation = false;
    int breathingStep = 0;
    int groundingStep = 0;
    int muscleStep = 0;
    float timer = 0.0f;
    bool exerciseActive = false;

    vector<string> breathingSteps = {
        "Breathe in quietly through your nose for 4 seconds...",
        "Hold your breath for 7 seconds...",
        "Exhale completely through your mouth for 8 seconds..."
    };

    vector<string> groundingSteps = {
        "Name 5 things you can see around you",
        "Name 4 things you can touch right now",
        "Name 3 things you can hear right now",
        "Name 2 things you can smell or like the smell of",
        "Name 1 thing you can taste or like the taste of"
    };

    vector<string> muscleGroups = {
        "Hands (clench fists)",
        "Arms (bend elbows and tense biceps)",
        "Shoulders (shrug them up)",
        "Face (scrunch all facial muscles)",
        "Stomach (tighten abs)",
        "Legs (press feet down and tense thighs)",
        "Feet (curl toes)"
    };

public:
    void renderExerciseMenu() {
        if (ImGui::Button("4-7-8 Breathing Exercise")) {
            showBreathingExercise = true;
            breathingStep = 0;
            timer = 0.0f;
            exerciseActive = true;
        }
        
        if (ImGui::Button("5-4-3-2-1 Grounding Exercise")) {
            showGroundingExercise = true;
            groundingStep = 0;
            timer = 0.0f;
            exerciseActive = true;
        }
        
        if (ImGui::Button("Progressive Muscle Relaxation")) {
            showMuscleRelaxation = true;
            muscleStep = 0;
            timer = 0.0f;
            exerciseActive = true;
        }
    }

    void updateExercises(float deltaTime) {
        if (!exerciseActive) return;

        timer += deltaTime;
        
        if (showBreathingExercise) {
            if (timer >= 4.0f && breathingStep == 0) {
                breathingStep = 1;
                timer = 0.0f;
            } else if (timer >= 7.0f && breathingStep == 1) {
                breathingStep = 2;
                timer = 0.0f;
            } else if (timer >= 8.0f && breathingStep == 2) {
                breathingStep = 0;
                timer = 0.0f;
                showBreathingExercise = false;
                exerciseActive = false;
            }
        }
        
        if (showGroundingExercise) {
            if (timer >= 5.0f) {
                groundingStep++;
                timer = 0.0f;
                if (groundingStep >= groundingSteps.size()) {
                    showGroundingExercise = false;
                    exerciseActive = false;
                }
            }
        }
        
        if (showMuscleRelaxation) {
            if (timer >= 5.0f) {
                muscleStep++;
                timer = 0.0f;
                if (muscleStep >= muscleGroups.size()) {
                    showMuscleRelaxation = false;
                    exerciseActive = false;
                }
            }
        }
    }

    void renderExercises() {
        if (showBreathingExercise) {
            ImGui::Begin("Breathing Exercise", &showBreathingExercise);
            ImGui::Text("Let's do a simple 4-7-8 breathing exercise to help you relax:");
            ImGui::Text("Current step: %s", breathingSteps[breathingStep].c_str());
            ImGui::ProgressBar(timer / (breathingStep == 0 ? 4.0f : (breathingStep == 1 ? 7.0f : 8.0f)));
            ImGui::End();
        }
        
        if (showGroundingExercise) {
            ImGui::Begin("Grounding Exercise", &showGroundingExercise);
            ImGui::Text("Let's do a 5-4-3-2-1 grounding exercise:");
            ImGui::Text("Current step: %s", groundingSteps[groundingStep].c_str());
            ImGui::ProgressBar(timer / 5.0f);
            ImGui::End();
        }
        
        if (showMuscleRelaxation) {
            ImGui::Begin("Muscle Relaxation", &showMuscleRelaxation);
            ImGui::Text("Let's try progressive muscle relaxation:");
            ImGui::Text("Current muscle group: %s", muscleGroups[muscleStep].c_str());
            ImGui::ProgressBar(timer / 5.0f);
            ImGui::End();
        }
    }
};

// Enhanced Assistant with ImGui
class Assistant {
private:
    User* user;
    MoodTracker moodTracker;
    CodeProvider codeProvider;
    CalmingActivityProvider calmingActivity;
    bool showMoodHistory = false;
    bool showQuote = false;
    string currentQuote = "";
    string currentMood = "";
    GLFWwindow* window; // <-- Add this line

public:
    Assistant(string name, GLFWwindow* win) { // <-- Add window parameter
        user = new User(name);
        window = win; // <-- Store window pointer
    }

    ~Assistant() {
        delete user;
    }

    void renderMainMenu() {
        ImGui::Begin("Mental Health Check-In");

        if (ImGui::CollapsingHeader("Mood Check-In")) {
            moodTracker.renderMoodSelection(user);
            
            if (moodTracker.isMoodSelected() && ImGui::Button("Submit Mood")) {
                currentMood = moodTracker.askMood(user);
                user->addMoodHistory(currentMood);
                currentQuote = codeProvider.getRandomQuote(currentMood);
                showQuote = true;
            }
        }

        if (ImGui::Button("View Mood History")) {
            showMoodHistory = true;
        }

        if (ImGui::CollapsingHeader("Relaxation Exercises")) {
            calmingActivity.renderExerciseMenu();
        }

        if (ImGui::Button("Exit")) {
            glfwSetWindowShouldClose(window, true); // Use the stored window pointer
        }

        ImGui::End();
    }

    void renderMoodHistory() {
        if (showMoodHistory) {
            ImGui::Begin("Mood History", &showMoodHistory);
            
            ImGui::Text("Your most common mood: %s", user->getMoodTrend().c_str());
            
            ImGui::Separator();
            ImGui::Text("Recent moods:");
            auto recentMoods = user->getRecentMoods(3);
            for (const auto& mood : recentMoods) {
                ImGui::BulletText("%s", mood.c_str());
            }
            
            ImGui::Separator();
            ImGui::Text("Full history:");
            auto allMoods = user->getMoodHistory();
            for (size_t i = 0; i < allMoods.size(); i++) {
                ImGui::Text("%d. %s", i+1, allMoods[i].c_str());
            }
            
            ImGui::End();
        }
    }

    void renderQuote() {
        if (showQuote) {
            ImGui::Begin("Daily Quote", &showQuote);
            ImGui::Text("You're feeling: %s", currentMood.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("%s", currentQuote.c_str());
            ImGui::End();
        }
    }

    void update(float deltaTime) {
        calmingActivity.updateExercises(deltaTime);
    }

    void render() {
        renderMainMenu();
        renderMoodHistory();
        renderQuote();
        calmingActivity.renderExercises();
    }
};

int main() {
    // Initialize GLFW
    if (!glfwInit())
        return 1;

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Mental Health Check-in Simulator", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Create assistant with default name
    Assistant assistant("User", window);

    // Time tracking for exercises
    float lastTime = glfwGetTime();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Calculate delta time
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Update and render application
        assistant.update(deltaTime);
        assistant.render();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
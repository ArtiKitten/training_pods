#include <Adafruit_SSD1306_RK.h>

SYSTEM_THREAD(ENABLED);

Adafruit_SSD1306 display(128, 32, &Wire, -1);

int state = 0;
int n_try = 0;

unsigned long start_time;
unsigned long stop_time;

int result[3];
int best_result = 0;
uint16_t all_time_best = 0;

// Configuration initiale
void setup() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
	display.clearDisplay();
	display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 17);
    display.println("Starting...");
    display.display();
    
    // Fonctions et variables cloud
    Particle.variable("all_time_best", all_time_best);
    Particle.function("clean_result", eraseBestResult);
    
    // Récupère le meilleur score en mémoire
    EEPROM.get(0, all_time_best);
    
    pinMode(D4, OUTPUT);
    pinMode(D7, INPUT);
    
    digitalWrite(D4, HIGH);
    delay(3000);
}

// Boucle principale
void loop() {
    if (state == 0){
        // État du menu principal
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Main menu: Get your");
        display.println("reaction time by");
        display.println("pressing the button!"); 
        display.printlnf("All time: %d ms", all_time_best);
        display.display();
        
        while (digitalRead(D7) == LOW);     // Détecte l'appuie
        while (digitalRead(D7) == HIGH);    // Détecte le relachement
        
        state += 1;                         // Incrémente l'état
        
        
        display.clearDisplay();
    }
    else if (state == 1 || state == 3 || state == 5){
        // État de jeu
        display.clearDisplay();
        display.setCursor(0, 0);
        display.printlnf("Tentative #%d", n_try+1);
        for (int i = 0; i < 3; i++){
            display.printlnf("- %d ms", result[i]);
        }
        display.display();
        
        delay(random(1500, 4000));
        
        digitalWrite(D4, LOW);              // Allume la DEL 
        start_time = micros();              // Démarre le chronomètre
        
        while (digitalRead(D7) == LOW);     // Détecte l'appuie
        stop_time = micros();               // Fin du chronomètre
        digitalWrite(D4, HIGH);             // Éteint la DEL
        while (digitalRead(D7) == HIGH);    // Détecte le relachement
        
        state += 1;                         // Incrémente l'état              
    }
    else if (state == 2 || state == 4 || state == 6){
        // État de résultats
        result[n_try] = (stop_time - start_time) / 1000;
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.printlnf("Tentative #%d", n_try+1);
        for (int i = 0; i < 3; i++){
            display.printlnf("- %d ms", result[i]);
        }
        display.display();
        
        if (result[n_try] < best_result || best_result == 0) 
            best_result = result[n_try];
        
        state += 1;                         // Incrémente l'état
        n_try += 1;                         // Incrémente le numéro de tentative
    }
    else if (state == 7){
        // État de fin du jeu
        
        // Validation du meilleur résultat et écriture en mémoire
        if (best_result < all_time_best || all_time_best == 0xFFFF) {
            all_time_best = best_result;
            EEPROM.put(0, best_result);
        }
        
        delay(1000);
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Well played!");
        display.println("Your best result is");
        display.printlnf("%d ms", best_result);
        display.display();
        
        while (digitalRead(D7) == LOW);     // Détecte l'appuie
        while (digitalRead(D7) == HIGH);    // Détecte le relachement
        
        // Réinitialise les compteurs et les scores temporaires
        for (int i = 0; i < 3; i++){
            result[i] = 0;
        }
        
        state = 0;
        n_try = 0;
        best_result = 0;
    }
}

// Fonction de réinitialisation de la mémoire (cloud)
int eraseBestResult(String _){
    EEPROM.put(0, 0xFFFF);
    return 0;
}

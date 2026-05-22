#include <avr/power.h>

#define PIN_BOUTON 4

bool modulesActifs = true;

void activerModules() {
    PRR &= ~(1 << PRADC);
    PRR &= ~(1 << PRSPI);
    PRR &= ~(1 << PRTWI);
    PRR &= ~(1 << PRTIM1);
    PRR &= ~(1 << PRTIM2);
}

void desactiverModules() {
    PRR |= (1 << PRADC);
    PRR |= (1 << PRSPI);
    PRR |= (1 << PRTWI);
    PRR |= (1 << PRTIM1);
    PRR |= (1 << PRTIM2);
}

void setup() {
    pinMode(PIN_BOUTON, INPUT_PULLUP);
    Serial.begin(9600);
    Serial.println("Modules ON");
}

void loop() {
    static bool dernierEtat = HIGH;
    bool etatActuel = digitalRead(PIN_BOUTON);

    // Détection front descendant (appui)
    if (dernierEtat == HIGH && etatActuel == LOW) {
        modulesActifs = !modulesActifs;

        if (modulesActifs) {
            activerModules();
            Serial.println("Modules ON");
        } else {
            desactiverModules();
            Serial.println("Modules OFF");
        }

        delay(50); // anti-rebond
    }

    dernierEtat = etatActuel;
}
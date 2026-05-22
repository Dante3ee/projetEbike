#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define PIN_LED 6        // Pin data pour les LED
#define NUMPIXELS 2      // Nb de LED

#define CAPTEUR 2            // Pin capteur effet Hall
#define DIAMETRE_ROUE 0.655  //Diamètre de la roue en m 

//Couleurs pour les LED
static constexpr float VITESSE_LED_ON   =  8.0f;
static constexpr float SEUIL_VITESSE    = 10.0f;
static constexpr float VITESSE_VERT_FIN = 11.0f;
static constexpr float VITESSE_JAUNE    = 14.0f;

volatile unsigned long nombreImpulsions = 0;  //déclaration du nombre d'impulsion
unsigned long tempsDebutMesure = 0;           
const unsigned long dureeMesure = 741;        // 0.741 secondes pour seuil à 10 km/h

float calculDeVitesse(float n, float deltaT) {
    float toursParSeconde = n / deltaT;
    float vitesse_m_s = toursParSeconde * PI * DIAMETRE_ROUE;
    float vitesse_km_h = vitesse_m_s * 3.6;
    return vitesse_km_h;
}

void compteImpulsion() {  // rajoute +1 une impulsion
  nombreImpulsions++;
}

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);  // NEO_GRB + NEO_KHZ800

void setup() {
    // Désactivation des modules inutilisés pour économiser de l'énergie
    PRR |= (1 << PRADC);   // désactive ADC
    PRR |= (1 << PRSPI);   // désactive SPI
    PRR |= (1 << PRTWI);   // désactive I2C
    PRR |= (1 << PRTIM1);  // désactive Timer1
    PRR |= (1 << PRTIM2);  // désactive Timer2

    pixels.begin();       // Initialise la lib des LED
    pinMode(CAPTEUR, INPUT_PULLUP);                                               //init resitance interne du capteur en position entrée
    attachInterrupt(digitalPinToInterrupt(CAPTEUR), compteImpulsion, FALLING);
    tempsDebutMesure = millis();                                                  // Le timer vaut 0 quand tout est "setup" LAISSER EN DERNIER DANS LE SETUP
}

void loop() {
  if (millis() - tempsDebutMesure >= dureeMesure) {
    float deltaT = dureeMesure / 1000.0;

    noInterrupts();
    unsigned long impulsionsLues = nombreImpulsions;
    interrupts();
    float vitesse = calculDeVitesse(impulsionsLues, deltaT);
    
    // LED 1 : Affichage de la vitesse 
    // Éteint tout sous 8 km/h
    // 8 → 10 km/h : fondue bleu -> vert
    // 10 → 11 km/h : vert pur
    // 11 → 14 km/h : fondue vert -> jaune
    // ≥ 14 km/h : jaune pur

    if (vitesse < VITESSE_LED_ON) {
        pixels.setPixelColor(0, 0);

    } else if (vitesse < SEUIL_VITESSE) {
        float t = (vitesse - VITESSE_LED_ON) / (SEUIL_VITESSE - VITESSE_LED_ON);
        pixels.setPixelColor(0, pixels.Color(0, t * 10, (1 - t) * 10));

    } else if (vitesse <= VITESSE_VERT_FIN) {
        pixels.setPixelColor(0, pixels.Color(0, 10, 0));

    } else if (vitesse < VITESSE_JAUNE) {
        float t = (vitesse - VITESSE_VERT_FIN) / (VITESSE_JAUNE - VITESSE_VERT_FIN);
        pixels.setPixelColor(0, pixels.Color(t * 10, 10, 0));

    } else {
        pixels.setPixelColor(0, pixels.Color(10, 10, 0));
    }

    // LED 2 : timer 2h  violet → bleu → vert → jaune → orange → rouge

    static unsigned long debutTimer = millis();
    static constexpr unsigned long INTERVALLE = 20UL * 60 * 1000; // 20 min en ms

    static const uint32_t palette[6][3] = {
        {10, 0, 10},  // violet
        { 0, 0, 10},  // bleu
        { 0, 10,  0}, // vert
        {10, 10,  0}, // jaune
        {10,  4,  0}, // orange
        {10,  0,  0}, // rouge
    };

    unsigned long elapsed = millis() - debutTimer;
    uint8_t etape =  min(elapsed / INTERVALLE, 5UL);     // 0 à 5 (Violet à rouge)
    float tTimer = (float)(elapsed % INTERVALLE) / INTERVALLE; // 0.0 -> 1.0

    if (etape >= 5) {
        // Temps écoulé : rouge fixe
        pixels.setPixelColor(1, pixels.Color(10, 0, 0));
    } else {
        // Fondue entre l'étape courante et la suivante
        uint8_t r = palette[etape][0] + tTimer * (palette[etape+1][0] - palette[etape][0]);
        uint8_t g = palette[etape][1] + tTimer * (palette[etape+1][1] - palette[etape][1]);
        uint8_t b = palette[etape][2] + tTimer * (palette[etape+1][2] - palette[etape][2]);
        pixels.setPixelColor(1, pixels.Color(r, g, b));
    }

    pixels.show();  //afficher les changements de couleurs

    // reset du timer et du nb d'impulsions A METTRE EN DERNIER DANS LA LOOP
    nombreImpulsions = 0;
    tempsDebutMesure = millis();
  }
}

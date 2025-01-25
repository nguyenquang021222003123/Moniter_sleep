#include <Arduino.h>
#line 1 "C:\\Users\\DELL\\Documents\\Arduino\\project\\Vi_xu_ly\\Vi_xu_ly.ino"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30100_PulseOximeter.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_Sensor.h>

// --- Thiết lập màn hình OLED ---
#define SCREEN_WIDTH 128 // Độ rộng của màn hình OLED
#define SCREEN_HEIGHT 64 // Độ cao của màn hình OLED
#define OLED_RESET -1    // Không sử dụng chân reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Thiết lập cảm biến MAX30100 (Pulse Oximeter) ---
PulseOximeter pox;

// --- Thiết lập cảm biến MPU6050 (Chuyển động) ---
Adafruit_MPU6050 mpu;

// --- Thiết lập cảm biến SHT31 (Nhiệt độ) ---
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// --- Thiết lập Microphone MAX9814 ---
#define MIC_PIN 35             // Chân analog kết nối với MAX9814 OUT
const unsigned long sampleWindow = 50; // Khoảng thời gian lấy mẫu tín hiệu (ms)
unsigned int signalMax = 0;
unsigned int signalMin = 4095;

// --- Thời gian cập nhật dữ liệu ---
const uint32_t REPORTING_PERIOD_MS = 1000; // Cập nhật nhịp tim mỗi giây
const uint32_t OTHER_SENSORS_PERIOD_MS = 500; // Cập nhật các cảm biến khác mỗi 500ms
unsigned long lastHeartRateMillis = 0;
unsigned long lastOtherSensorsMillis = 0;

// --- Callback khi phát hiện nhịp tim ---
// void onBeatDetected()
// {
//     Serial.println("BPM detected!");
// }

#line 42 "C:\\Users\\DELL\\Documents\\Arduino\\project\\Vi_xu_ly\\Vi_xu_ly.ino"
void setup();
#line 105 "C:\\Users\\DELL\\Documents\\Arduino\\project\\Vi_xu_ly\\Vi_xu_ly.ino"
void loop();
#line 42 "C:\\Users\\DELL\\Documents\\Arduino\\project\\Vi_xu_ly\\Vi_xu_ly.ino"
void setup()
{
    // Khởi động Serial
    Serial.begin(115200);

    // Khởi động giao tiếp I2C với chân SDA = 21 và SCL = 22
    Wire.begin(21, 22);

    // Khởi động màn hình OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Địa chỉ I2C thường là 0x3C
        Serial.println(F("SSD1306 allocation failed"));
        while (true); // Dừng chương trình nếu OLED không khởi động được
    }
    delay(1000);
    display.clearDisplay();
    display.setTextSize(1);  // Đồng bộ cỡ chữ
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();

    // Khởi động cảm biến MPU6050
    if (!mpu.begin()) {
        Serial.println("MPU6050 không tìm thấy!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MPU6050 not found!");
        display.display();
        while (true); // Dừng chương trình nếu MPU6050 không khởi động được
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Khởi động cảm biến SHT31
    if (!sht31.begin(0x44)) { // Địa chỉ mặc định của SHT31 là 0x44
       Serial.println("SHT31 không tìm thấy!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("SHT31 not found!");
        display.display();
        while (true); // Dừng chương trình nếu SHT31 không khởi động được
    }

    // Khởi động cảm biến MAX30100 (Pulse Oximeter)
    if (!pox.begin()) {
        Serial.println("Failed to initialize PulseOximeter");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("PulseOximeter failed!");
        display.display();
        while (true); // Dừng chương trình nếu MAX30100 không khởi động được
    }

    //pox.setOnBeatDetectedCallback(onBeatDetected);

    // Hiển thị thông báo hoàn thành khởi tạo
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Initialization done!");
    display.display();
}

void loop()
{
    unsigned long currentMillis = millis();

    // --- Cập nhật dữ liệu từ cảm biến Pulse Oximeter ---
    pox.update();
    if (currentMillis - lastHeartRateMillis > REPORTING_PERIOD_MS) {
        lastHeartRateMillis = currentMillis;

        // Lấy nhịp tim hiện tại
        float bpm = pox.getHeartRate();

        // In ra Serial Monitor
        Serial.print("Heart Rate: ");
        Serial.print(bpm);
        Serial.println(" BPM");

        float temperature = sht31.readTemperature();
        float humidity = sht31.readHumidity();

        // In ra Serial Monitor
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" C");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");

        int peakToPeak = signalMax - signalMin;
        float soundLevel = (float)peakToPeak * 3.3 / 4095.0;

        // In ra Serial Monitor
        //Serial.print("Sound Level: ");
        Serial.print(soundLevel);
        Serial.println(" V");

        // Lấy dữ liệu từ cảm biến MPU6050
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // In dữ liệu MPU6050 ra Serial Monitor
        Serial.print("Accel (X, Y, Z): ");
        Serial.print(a.acceleration.x);
        Serial.print(", ");
        Serial.print(a.acceleration.y);
        Serial.print(", ");
        Serial.print(a.acceleration.z);
        Serial.println(" m/s^2");

        Serial.print("Gyro (X, Y, Z): ");
        Serial.print(g.gyro.x);
        Serial.print(", ");
        Serial.print(g.gyro.y);
        Serial.print(", ");
        Serial.print(g.gyro.z);
        Serial.println(" rad/s");

        // Hiển thị dữ liệu từ các cảm biến lên màn hình OLED
        display.clearDisplay();
        display.setCursor(0, 16);
        display.print("Temp: ");
        display.print(temperature);
        display.println(" C");

        display.print("Humidity: ");
        display.print(humidity);
        display.println(" %");

        display.print("Sound: ");
        display.print(soundLevel);
        display.println(" V");

        display.print("Accel: ");
        display.print(a.acceleration.x, 1);
        display.print(",");
        display.print(a.acceleration.y, 1);
        display.print(",");
        display.print(a.acceleration.z, 1);
        display.println(" m/s^2");

        display.print("Gyro: ");
        display.print(g.gyro.x, 1);
        display.print(",");
        display.print(g.gyro.y, 1);
        display.print(",");
        display.print(g.gyro.z, 1);
        display.println(" rad/s");
    }

    // --- Cập nhật dữ liệu từ các cảm biến khác ---
    if (currentMillis - lastOtherSensorsMillis >= OTHER_SENSORS_PERIOD_MS) {
        lastOtherSensorsMillis = currentMillis;

        // Đọc tín hiệu từ MAX9814 (microphone)
        unsigned long startMillis = millis();
        signalMax = 0;
        signalMin = 4095;

        while (millis() - startMillis < sampleWindow) {
            int sample = analogRead(MIC_PIN);
            if (sample > signalMax) signalMax = sample;
            if (sample < signalMin) signalMin = sample;
        }

        unsigned int peakToPeak = signalMax - signalMin;
        float voltage = (peakToPeak * 3.3) / 4095.0;

        // Đọc dữ liệu từ MPU6050
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Đọc nhiệt độ từ SHT31
        float temperature = sht31.readTemperature();

        // --- Hiển thị dữ liệu đồng bộ trên OLED ---
        display.clearDisplay();
        display.setTextSize(1); // Đồng bộ cỡ chữ
        display.setTextColor(SSD1306_WHITE);

        // Hiển thị nhịp tim
        display.setCursor(0, 0);
        display.print("HR: ");
        if (pox.getHeartRate() < 0)
            display.println("----");
        else
            display.println((int)pox.getHeartRate());

        // Hiển thị mức âm thanh
        display.print("Sound: ");
        display.print(voltage, 1);
        display.println(" V");
        if (voltage > 1.5) { // Ngưỡng tiếng ngáy
            display.println("Snore detected!");
        }

        // Hiển thị nhiệt độ
        display.print("Temp: ");
        display.print(temperature, 1);
        display.println(" C");

        // Hiển thị gia tốc
        display.print("Acc: ");
        display.print(a.acceleration.x, 1);
        display.print(", ");
        display.print(a.acceleration.y, 1);
        display.print(", ");
        display.println(a.acceleration.z, 1);

        // Hiển thị gia tốc góc
        display.print("Gyro: ");
        display.print(g.gyro.x, 1);
        display.print(", ");
        display.print(g.gyro.y, 1);
        display.print(", ");
        display.print(g.gyro.z, 1);

        display.display();
    }
}


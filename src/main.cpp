#include <Arduino.h>
#include <M5Unified.h>
#include <M5ModuleLLM.h>
#include <ArduinoJson.h>

M5ModuleLLM module_llm;

String vad_work_id;
String whisper_work_id;
String language = "ja";

bool is_recording = false;
bool is_initialized = false;

int button_x, button_y, button_radius;

void drawRecordButton(bool pressed) {
    int center_x = M5.Display.width() / 2;
    int center_y = M5.Display.height() / 2;
    int radius = 40;
    
    button_x = center_x;
    button_y = center_y;
    button_radius = radius;
    
    if (pressed) {
        M5.Display.fillCircle(center_x, center_y, radius, TFT_MAROON);
        M5.Display.fillCircle(center_x, center_y, radius - 5, TFT_RED);
        M5.Display.setTextColor(TFT_WHITE, TFT_RED);
        M5.Display.setTextDatum(middle_center);
        M5.Display.drawString("REC", center_x, center_y);
    } else {
        M5.Display.fillCircle(center_x, center_y, radius, TFT_DARKGREY);
        M5.Display.fillCircle(center_x, center_y, radius - 5, TFT_RED);
        M5.Display.setTextColor(TFT_WHITE, TFT_RED);
        M5.Display.setTextDatum(middle_center);
        M5.Display.drawString("TAP", center_x, center_y);
    }
    M5.Display.setTextDatum(top_left);
}

void startRecording() {
    if (!is_recording && is_initialized) {
        is_recording = true;
        drawRecordButton(true);
        
        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.printf("録音開始...\n");
        
        m5_module_llm::ApiVadSetupConfig_t vad_config;
        vad_config.input = {"sys.pcm"};
        vad_work_id = module_llm.vad.setup(vad_config, "vad_setup");
        
        m5_module_llm::ApiWhisperSetupConfig_t whisper_config;
        whisper_config.input = {"sys.pcm", vad_work_id};
        whisper_config.language = language;
        whisper_work_id = module_llm.whisper.setup(whisper_config, "whisper_setup");
    }
}

void stopRecording() {
    if (is_recording) {
        is_recording = false;
        drawRecordButton(false);
        
        M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
        M5.Display.setCursor(10, 30);
        M5.Display.printf("録音停止\n");
    }
}

void setup() {
    M5.begin();
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.clear();
    M5.Display.setFont(&fonts::efontJA_12);

    M5.Display.printf("M5Stack 日本語音声認識システム\n");
    M5.Display.printf("================\n");

    int rxd = M5.getPin(m5::pin_name_t::port_c_rxd);
    int txd = M5.getPin(m5::pin_name_t::port_c_txd);
    Serial2.begin(115200, SERIAL_8N1, rxd, txd);

    module_llm.begin(&Serial2);

    M5.Display.printf(">> ModuleLLM接続確認中...\n");
    while (1) {
        if (module_llm.checkConnection()) {
            break;
        }
        delay(100);
    }
    M5.Display.printf(">> 接続完了\n");

    M5.Display.printf(">> ModuleLLMリセット中...\n");
    module_llm.sys.reset();
    delay(2000);

    M5.Display.printf(">> 音響システム初期化中...\n");
    module_llm.audio.setup();

    M5.Display.printf(">> システム初期化完了\n");
    delay(1000);
    
    M5.Display.clear();
    M5.Display.setCursor(10, 10);
    M5.Display.printf("録音ボタンをタップしてください\n");
    
    drawRecordButton(false);
    is_initialized = true;
}

void loop() {
    module_llm.update();
    M5.update();

    if (is_recording) {
        for (auto& msg : module_llm.msg.responseMsgList) {
            if (msg.work_id == vad_work_id) {
                M5.Display.setTextColor(TFT_BLUE, TFT_BLACK);
                M5.Display.setCursor(10, 50);
                M5.Display.printf("音声検出中...\n");
            }

            if (msg.work_id == whisper_work_id) {
                if (msg.object == "asr.utf-8") {
                    JsonDocument doc;
                    deserializeJson(doc, msg.raw_msg);
                    String asr_result = doc["data"].as<String>();

                    if (asr_result.length() > 0) {
                        M5.Display.fillRect(0, 200, M5.Display.width(), 40, TFT_BLACK);
                        M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
                        M5.Display.setCursor(10, 200);
                        M5.Display.printf("認識: %s", asr_result.c_str());
                    }
                }
            }
        }
    }

    module_llm.msg.responseMsgList.clear();

    if (M5.Touch.getCount()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            int touch_x = touch.x;
            int touch_y = touch.y;
            
            int distance = sqrt(pow(touch_x - button_x, 2) + pow(touch_y - button_y, 2));
            
            if (distance <= button_radius) {
                if (!is_recording) {
                    startRecording();
                } else {
                    stopRecording();
                }
            }
        }
    }

    if (M5.BtnA.wasPressed()) {
        M5.Display.clear();
        M5.Display.setCursor(10, 10);
        M5.Display.printf("録音ボタンをタップしてください\n");
        drawRecordButton(is_recording);
    }

    if (M5.BtnB.wasPressed()) {
        M5.Display.printf(">> システム再起動\n");
        ESP.restart();
    }

    if (M5.BtnC.wasPressed()) {
        if (is_recording) {
            stopRecording();
        }
        M5.Display.setCursor(10, 220);
        M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Display.printf("録音状態: %s", is_recording ? "ON" : "OFF");
    }

    delay(10);
}

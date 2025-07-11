#include <Arduino.h>
#include <M5Unified.h>
#include <M5ModuleLLM.h>
#include <ArduinoJson.h>

M5ModuleLLM module_llm;

String wake_up_keyword = "HELLO";
String kws_work_id;
String vad_work_id;
String whisper_work_id;
String language = "ja";

void setup() {
    M5.begin();
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextScroll(true);
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

    M5.Display.printf(">> キーワード検出設定中...\n");
    m5_module_llm::ApiKwsSetupConfig_t kws_config;
    kws_config.kws = wake_up_keyword;
    kws_work_id = module_llm.kws.setup(kws_config, "kws_setup", language);

    M5.Display.printf(">> 音声活動検出設定中...\n");
    m5_module_llm::ApiVadSetupConfig_t vad_config;
    vad_config.input = {"sys.pcm", kws_work_id};
    vad_work_id = module_llm.vad.setup(vad_config, "vad_setup");

    M5.Display.printf(">> 日本語音声認識設定中...\n");
    m5_module_llm::ApiWhisperSetupConfig_t whisper_config;
    whisper_config.input = {"sys.pcm", kws_work_id, vad_work_id};
    whisper_config.language = language;
    whisper_work_id = module_llm.whisper.setup(whisper_config, "whisper_setup");

    M5.Display.printf(">> システム開始\n");
    M5.Display.printf("'%s'と話しかけてください\n", wake_up_keyword.c_str());
    M5.Display.printf("================\n");
}

void loop() {
    module_llm.update();
    M5.update();

    for (auto& msg : module_llm.msg.responseMsgList) {
        if (msg.work_id == kws_work_id) {
            M5.Display.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
            M5.Display.printf(">> キーワード検出: %s\n", wake_up_keyword.c_str());
            M5.Display.printf(">> 音声認識開始...\n");
        }

        if (msg.work_id == vad_work_id) {
            M5.Display.setTextColor(TFT_BLUE, TFT_BLACK);
            M5.Display.printf(">> 音声活動検出\n");
        }

        if (msg.work_id == whisper_work_id) {
            if (msg.object == "asr.utf-8") {
                JsonDocument doc;
                deserializeJson(doc, msg.raw_msg);
                String asr_result = doc["data"].as<String>();

                M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
                M5.Display.printf(">> 認識結果: %s\n", asr_result.c_str());

                if (asr_result.length() > 0) {
                    M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
                    M5.Display.printf("日本語音声: %s\n", asr_result.c_str());
                }
            }
        }
    }

    module_llm.msg.responseMsgList.clear();

    if (M5.BtnA.wasPressed()) {
        M5.Display.clear();
        M5.Display.setCursor(0, 0);
        M5.Display.printf("画面クリア完了\n");
        M5.Display.printf("'%s'と話しかけてください\n", wake_up_keyword.c_str());
    }

    if (M5.BtnB.wasPressed()) {
        M5.Display.printf(">> システム再起動\n");
        ESP.restart();
    }

    if (M5.BtnC.wasPressed()) {
        M5.Display.printf(">> システム情報\n");
        M5.Display.printf("言語: %s\n", language.c_str());
        M5.Display.printf("キーワード: %s\n", wake_up_keyword.c_str());
        M5.Display.printf("KWS ID: %s\n", kws_work_id.c_str());
        M5.Display.printf("VAD ID: %s\n", vad_work_id.c_str());
        M5.Display.printf("Whisper ID: %s\n", whisper_work_id.c_str());
    }

    delay(10);
}

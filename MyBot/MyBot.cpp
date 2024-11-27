#include "MyBot.h"
#include <dpp/dpp.h>
#include <fstream>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>


std::time_t snowflake_to_unix_timestamp(dpp::snowflake id) {
	const int64_t DISCORD_EPOCH = 1420070400000; // Discord 紀元時間
	return (id >> 22) + DISCORD_EPOCH;
}


void time_fuction(dpp::snowflake id,char* time_buffer) {
	std::time_t unix_timestamp = snowflake_to_unix_timestamp(id)/1000;
	errno_t err = ctime_s(time_buffer, 50, &unix_timestamp);
	if (err != 0) {
		std::cerr << "ctime_s failed with error code: " << err << std::endl;
		return;}
	size_t time_buffer_len = strlen(time_buffer);
	if (time_buffer_len > 0 && time_buffer[time_buffer_len - 1] == '\n') {
		time_buffer[time_buffer_len - 1] = '\0';
	}// 手動去掉換行符
}


int main()
{
	SetConsoleOutputCP(CP_UTF8);
	/* 從本地讀取bot_token */
	std::ifstream token_txt("bot_token.txt");
	std::string  temp_token;
	if (token_txt.is_open()) {
		token_txt >> temp_token;
		token_txt.close();
	} else {
		std::cerr << "無法打開檔案！" << std::endl;
	}
	const std::string    BOT_TOKEN = temp_token;


	/* 連線至Discord Bot */
	uint32_t intents = dpp::i_default_intents | dpp::i_message_content;
	dpp::cluster bot(BOT_TOKEN , intents);


	/* 輸出簡單日誌訊息至控制台 */
	bot.on_log(dpp::utility::cout_logger());


	/* 註冊斜線指令在這裡 */
	bot.on_ready([&bot](const dpp::ready_t& event) {
		/* 將註冊斜線命令只跑一次，以確保它不會在每次完全重新連接時運行 */
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands {
				{ "ping", "查看當前延遲", bot.me.id }
			};

			bot.global_bulk_command_create(commands);
		}
	});

	/* 使用最新新增的 D++ 功能、協程來處理斜線指令！ */
	bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (event.command.get_command_name() == "ping") {
			
			// 1. Bot 接收到指令的時間
			auto now = std::chrono::system_clock::now();
			auto timestamp_received = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
			// 2. 將 Snowflake ID 轉換為 Unix 時間戳（毫秒）
			std::time_t command_timestamp = snowflake_to_unix_timestamp(event.command.id);
			// 計算網絡延遲（用戶發送指令的時間 - bot 接收到的時間）
			long long network_latency;
			if (command_timestamp > timestamp_received) {
				network_latency = command_timestamp - timestamp_received;
			}
			else {
				network_latency = timestamp_received- command_timestamp;
			}
			// 發送初步回應
			co_await event.co_reply("處理中...");
			// 4. 計算延遲（Bot 收到指令到回應的時間差）
			auto now_after_reply = std::chrono::system_clock::now();
			auto timestamp_replied = std::chrono::time_point_cast<std::chrono::milliseconds>(now_after_reply).time_since_epoch().count();
			long long bot_latency = timestamp_replied - timestamp_received;
			// 5. 編輯回應
			std::string response = "Bot 延遲: " + std::to_string(bot_latency) + " 毫秒\n網絡延遲: " + std::to_string(network_latency) + " 毫秒";
			// 回應延遲時間
			co_await event.co_edit_response(response);
		}
		co_return;
	});

	//接收訊息
	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		/*變數區*/
		std::string bot_mention = "<@" + std::to_string(bot.me.id) + ">";
		char message_time[50];
		time_fuction(event.msg.id, message_time);
		

		/*輸出使用者訊息至控制台*/
		std::cout <<"["<< message_time << "] " << event.msg.author.username << " :" << event.msg.content << std::endl;
		std::ofstream file("output.txt", std::ios::app);
		if (file.is_open()) {
			file <<"["<<message_time << "] " << event.msg.content << "\n";
			file.close();
		}
		else {
			std::cerr << "無法打開檔案！" << std::endl;
		}
		});

	// 處理訊息編輯事件
	bot.on_message_update([&bot](const dpp::message_update_t& event) {
		char message_time[50];
		if (event.msg.edited) {
			/*變數區*/
			std::string bot_mention = "<@" + std::to_string(bot.me.id) + ">";
			time_fuction(event.msg.id, message_time);

			/* 輸出編輯後的訊息至控制台 */
			std::cout << "[" << message_time << "] "
				<< event.msg.author.username << ": " << event.msg.content <<" (已編輯)" << std::endl;
		}

		// 如果需要將編輯後的內容輸出到檔案，也可以加入這部分
		std::ofstream file("output.txt", std::ios::app);
		if (file.is_open()) {
			file <<"["<< message_time << "] " << event.msg.content << "\n";
			file.close();
		}
		else {
			std::cerr << "無法打開檔案！" << std::endl;
		}
		});
	/* 啟動機器人 */
	bot.start(dpp::st_wait);
	return 0;
}

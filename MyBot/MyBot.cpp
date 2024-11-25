#include "MyBot.h"
#include <dpp/dpp.h>

/* Be sure to place your token in the line below.
 * Follow steps here to get a token:
 * https://dpp.dev/creating-a-bot-application.html
 * When you invite the bot, be sure to invite it with the 
 * scopes 'bot' and 'applications.commands', e.g.
 * https://discord.com/oauth2/authorize?client_id=940762342495518720&scope=bot+applications.commands&permissions=139586816064
 */
const std::string    BOT_TOKEN    = "MTE4Mzc1NDI1NzY3NDAyNzA1OA.GeF2za.W8HyM0EsqJXcYfHWJJst0wb8OnJLYlgikSSoSs";

int main()
{
	/* 連線至Discord Bot */
	dpp::cluster bot(BOT_TOKEN);

	/* 輸出簡單日誌訊息至控制台 */
	bot.on_log(dpp::utility::cout_logger());

	/* 註冊斜線指令在這裡 */
	bot.on_ready([&bot](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands {
				{ "ping", "Ping pong!", bot.me.id }
			};

			bot.global_bulk_command_create(commands);
		}
	});

	/* 接收斜線指令並執行動作Handle slash command with the most recent addition to D++ features, coroutines! */
	bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (event.command.get_command_name() == "ping") {
			co_await event.co_reply("Pong!");
		}
		co_return;
	});

	/* 啟動機器人 */
	bot.start(dpp::st_wait);

	return 0;
}

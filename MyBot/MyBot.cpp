#include "MyBot.h"
#include <dpp/dpp.h>
#include<fstream>
#include<iostream>
std::ifstream in;
std::ofstream out;
/* Be sure to place your token in the line below.
 * Follow steps here to get a token:
 * https://dpp.dev/creating-a-bot-application.html
 * When you invite the bot, be sure to invite it with the 
 * scopes 'bot' and 'applications.commands', e.g.
 * https://discord.com/oauth2/authorize?client_id=940762342495518720&scope=bot+applications.commands&permissions=139586816064
 */


int main()
{
	in.open("bot_token.txt");
	if (in.fail()) {
		std::cout << "Bot Token load fail!";
	}
	std::string temp_token;
	in >> temp_token;
	const std::string    BOT_TOKEN = temp_token;
	/* �s�u��Discord Bot */
	dpp::cluster bot(BOT_TOKEN);

	/* ��X²���x�T���ܱ���x */
	bot.on_log(dpp::utility::cout_logger());

	/* ���U�׽u���O�b�o�� */
	bot.on_ready([&bot](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands {
				{ "ping", "Ping pong!", bot.me.id }
			};

			bot.global_bulk_command_create(commands);
		}
	});

	/* �����׽u���O�ð���ʧ@Handle slash command with the most recent addition to D++ features, coroutines! */
	bot.on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (event.command.get_command_name() == "ping") {
			co_await event.co_reply("Pong!");
		}
		co_return;
	});

	/* �Ұʾ����H */
	bot.start(dpp::st_wait);

	return 0;
}

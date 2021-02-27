#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <uWebSockets/App.h>
#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <algorithm>
#include <uWebSockets/WebSocket.h>
#include <uWebSockets/WebSocketProtocol.h>

using json = nlohmann::json;


struct UserData {
    size_t user_id;
    std::string user_name;
    std::string user_password;
    bool authorised;
};

class error {
public:
    json action_type_error = {
        {"err_code", 1},
        {"err_msg", "unknown action type"}
    };

    json parameter_required = {
        {"err_code", 2},
        {"err_msg", "one of parameters missed or invalid"}
        // так как это ВК сделало (на пофиг)
    };

    json invalid_opcode = {
        {"err_code", 3},
        {"err_msg", "only text msgs availiable"}
    };

    json incorrect_password = {
        {"err_code", 4},
        {"err_msg", "password is incorrect"}
    };

    json user_not_exists = {
        {"err_code", 5},
        {"err_msg", "user thith that id is not exists"}
    };

    json auth_required = {
        {"err_code", 6},
        {"err_msg", "Autorisation required"}
    };
};

size_t last_user_id = 0;
std::set<size_t> active_users;

// это бы всё в базу данных по-хорошему, ну ладно
std::map<size_t, UserData> users;


void regUser(UserData* userData, json &data) {
    std::string user_name, user_password;

    user_name = data["user_name"].get<std::string>();
    user_password = data["user_password"].get<std::string>();
    if (user_name.size() > 255 || user_password.size() > 25) {
        throw std::runtime_error("param_invalid");
    }

    userData->user_id = ++last_user_id;
    userData->user_name = user_name;
    userData->user_password = user_password;
    userData->authorised = true;

    active_users.insert(userData->user_id);
    users[userData->user_id] = *userData;
}

void authUser(UserData* userData, json &data) {
    std::string user_password;
    size_t user_id;
    user_id = data["id"].get<size_t>();
    user_password = data["user_password"].get<std::string>();

    if (user_id > last_user_id) {
        throw std::runtime_error("wrong_id");
    }

    if (users[user_id].user_password != user_password) {
        throw std::runtime_error("password_incorrect");
    }

    auto user = users[user_id];

    userData->user_id = user_id;
    userData->user_name = user.user_name;
    userData->user_password = user.user_password;
    userData->authorised = true;
}

int main() {

    error app_errors = error();

    uWS::App().ws<UserData>("/*", {
        .idleTimeout = 1200,
        .open = [](auto *ws) {

            auto userData = (UserData*)ws->getUserData();
            userData->authorised = false;

            json auth_json = {
                {"status", "connected"},
                {"info", "auth_required"}
            };

            ws->send(auth_json.dump(), uWS::OpCode::TEXT, true);
            std::cout << "New user connected\n"; 
        },
        .message = [&app_errors](auto *ws, auto message, uWS::OpCode opCode) {
            if (opCode != uWS::OpCode::TEXT) {
                ws->send(app_errors.invalid_opcode.dump(), uWS::OpCode::TEXT);
            }

            json data = json::parse(message); 

            std::string action_type;
            size_t user_id;
            // json схемы на плюсах я пилить не хочу, просто проверю так вот
            try {
                action_type = data["action"].get<std::string>();
                // user_id = data["id"].get<size_t>();
                auto userData = (UserData*)ws->getUserData();
                if (action_type == "reg") {
                    regUser(userData, data);

                    json success_json = {
                        {"status", "ok"},
                        {"info", "registered"},
                        {"id", userData->user_id}
                    };

                    ws->subscribe("user#"+std::to_string(userData->user_id));
                    ws->subscribe("all_users");

                    ws->send(success_json.dump(), opCode, true);

                    std::cout << "User " << userData->user_id << " registered\n"; 

                } else if (action_type == "auth") {
                    authUser(userData, data);
                    json success_json = {
                        {"status", "ok"},
                        {"info", "authorised"}
                    };

                    ws->subscribe("user#"+std::to_string(userData->user_id));
                    ws->subscribe("all_users");

                    ws->send(success_json.dump(), opCode, true);
                    std::cout << "User " << userData->user_id << " authorised\n"; 

                } else if (action_type == "msg") {
                    if (!userData->authorised) {
                        throw std::runtime_error("auth_reqiured");
                    }

                    std::string msg_text = data["message"].get<std::string>();
                    bool msg_to_all = data["msg_to_all"].get<bool>();

                    json new_msg = {
                        {"status", "ok"},
                        {"info", "msg_to_user"},
                        {"msg", msg_text},
                        {"author_id", userData->user_id},
                        {"author_name", userData->user_name}
                    };

                    if (!msg_to_all) {
                        size_t recv_id = data["recv_id"].get<size_t>();

                        if (recv_id > last_user_id) {
                            throw std::runtime_error("id_invalid");
                        }

                        ws->publish("user#"+std::to_string(recv_id), new_msg.dump(), opCode);
                        std::cout << "User " << userData->user_id << " sent msg to user " << recv_id << "\n"; 
                    } else {
                        ws->publish("all_users", new_msg.dump(), opCode);
                        std::cout << "User " << userData->user_id << " sent msg to everyone\n"; 
                    }
                } else {
                    std::cout << "Nothing_to_do\n";
                }
            } catch (json::exception) {
                ws->send(app_errors.parameter_required.dump(), opCode);
            } catch (std::runtime_error e) {
                // не буду выпендриваться с разными ошибками
                if (strcmp(e.what(), "param_invalid") == 0)
                    ws->send(app_errors.parameter_required.dump(), opCode);
                else if (strcmp(e.what(), "password_incorrect") == 0)
                    ws->send(app_errors.incorrect_password.dump(), opCode);
                else if (strcmp(e.what(), "id_invalid") == 0)
                    ws->send(app_errors.user_not_exists.dump(), opCode);
                else if (strcmp(e.what(), "auth_reqiured") == 0)
                    ws->send(app_errors.auth_required.dump(), opCode);
            }
        },
        .close = [](auto* ws, int code, auto message) {
            auto userData = (UserData*)ws->getUserData();
            active_users.erase(userData->user_id);

            std::cout << "User " << userData->user_id << " is offline now\n"; 
        }
        
    }).listen(9001, [](auto *listenSocket) {

        if (listenSocket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
        
    }).run();

    return 0;
}
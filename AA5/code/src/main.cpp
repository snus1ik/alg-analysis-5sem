#include <fstream>
#include <string>
#include <regex>
#include <iostream>
#include <queue>
#include <thread>
#include <filesystem>
#include <utility>
#include <mutex>
#include <chrono>
#include <sqlite3.h>
#include <algorithm>
#include <iterator>
#include <sstream>

std::mutex mtx_after_gen;
std::mutex mtx_after_read;
std::mutex mtx_after_extr;
std::mutex mtx_after_write;

int id_global = 0;
int id_issue = 9257;

struct timings {
    std::chrono::time_point<std::chrono::high_resolution_clock> start_gen;
    std::chrono::time_point<std::chrono::high_resolution_clock> pushed_a_gen;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_read;
    std::chrono::time_point<std::chrono::high_resolution_clock> pushed_a_read;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_extr;
    std::chrono::time_point<std::chrono::high_resolution_clock> pushed_a_extr;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_write;
    std::chrono::time_point<std::chrono::high_resolution_clock> pushed_a_write;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_destr;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_destr;
};

struct time_on_stage {
    //время существования задачи
    double time_existence;
    //среднее время обработки на каждой стадии
    double time_on_gen;
    double time_on_read;
    double time_on_extr;
    double time_on_write;
    double time_on_destr;
    //среднее время ожидания в каждой из очередей
    double time_wait_a_gen;
    double time_wait_a_read;
    double time_wait_a_extr;
    double time_wait_a_write;
};

class task {
public:
    int id;
    int issue_id;
    std::string filename;
    std::string html;
    std::string recipe_title;
    std::vector<std::string> recipe_ingredients;
    std::vector<std::string> recipe_steps;
    std::string recipe_img_url;
    bool is_last;  //поле, означающее, что элемент является последним в очереди и потоку не нужно ожидать следующие
    timings t;
    task() = delete;
    task(std::string file, const bool is_l): id(id_global++), issue_id(id_issue), filename(std::move(file)), is_last(is_l) {}
    ~task() = default;
};

double time_points_to_sec(std::chrono::high_resolution_clock::time_point fir,
    std::chrono::high_resolution_clock::time_point sec) {
    std::chrono::duration<double> diff = sec - fir;
    return diff.count();
}

void split(std::string str, const std::vector<std::string>& delimiters , std::vector<std::string>& tokens)
{
    for (auto& delim : delimiters) {
        std::size_t pos;
        while ((pos = str.find(delim)) != std::string::npos) {
            str.replace(pos, delim.size(), "|");
        }
    }
    std::string::size_type lastPos = str.find_first_not_of('|', 0);
    std::string::size_type pos = str.find_first_of('|', lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos){
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of('|', pos);
        pos = str.find_first_of('|', lastPos);
    }
}
bool predicate(const std::string &el) {
    return el.find(':') != std::string::npos;
}

std::vector<std::string> extract_ingredients(const std::string& html) {
    std::vector<std::string> ingr = {};
    std::size_t pos_start = html.find("<div class=\"recipe-text\">");
    if (pos_start == std::string::npos)
        return ingr;
    pos_start = html.find("<p>", pos_start);
    if (pos_start == std::string::npos)
        return ingr;
    std::size_t pos_end = html.find("</div>", pos_start);
    if (pos_end == std::string::npos)
        return ingr;
    pos_end = html.rfind("</p>", pos_end);
    if (pos_end == std::string::npos)
        return ingr;
    std::string data = html.substr(pos_start, pos_end - pos_start);
    std::vector<std::string> delims = {"<p>", "<br />", "</p>", "<ol>", "</ol>"};
    split(data, delims, ingr);
    std::erase_if(ingr, predicate);
    return ingr;
}

std::vector<std::string> extract_steps(const std::string &html) {
    std::vector<std::string> recipe = {};
    std::size_t pos_start = html.find("<h2 class=\"title uppercase\">Способ приготовления</h2>");
    if (pos_start == std::string::npos)
        return recipe;
    pos_start = html.find("<p>", pos_start);
    if (pos_start == std::string::npos)
        return recipe;
    std::size_t pos_end = html.find("<div>", pos_start);
    if (pos_end == std::string::npos)
        return recipe;
    pos_end = html.rfind("</p>", pos_end);
    std::string data = html.substr(pos_start, pos_end - pos_start);
    std::vector<std::string> delims = {"<p>", "</p>", "<br />", "<ol>", "</ol>"};
    split(data, delims, recipe);
    std::erase_if(recipe, predicate);
    return recipe;
}

std::string extract_title(const std::string &html) {
    std::size_t pos_start = html.find("<title>");
    if (pos_start == std::string::npos)
        return "";
    pos_start += 7;
    std::size_t pos_end = html.find("</title>", pos_start);
    if (pos_end == std::string::npos)
        return "";
    return html.substr(pos_start, pos_end - pos_start);
}

std::string extract_img_url(const std::string &html) {
    std::size_t pos_start = html.find("og:image");
    if (pos_start == std::string::npos)
        return "";
    pos_start = html.find("content", pos_start);
    if (pos_start == std::string::npos)
        return "";
    pos_start = html.find('\"', pos_start);
    if (pos_start == std::string::npos)
        return "";
    std::size_t pos_end = html.find('\"', pos_start + 1);
    if (pos_end == std::string::npos)
        return "";
    return html.substr(pos_start + 1, pos_end - pos_start - 1);
}

std::string get_data(const std::string &file_name) {
    std::string data;
    std::string line;
    std::ifstream fp(file_name);
    if (!fp)
        return "";
    while (std::getline(fp, line))
        data += line;
    return data;
}

void generator(const std::string &target_dir, std::queue<std::shared_ptr<task>> &after_gen_queue) {
    auto iter = std::filesystem::directory_iterator(target_dir);
    std::shared_ptr<task> t;
    /* Итерация по всем файлам в директории и помещение их в первую очередь конвейера
     * Если файл последний в директории, помечаем его последним через поле task.is_last
     */
    for (auto i = begin(iter); begin(iter) != end(iter); ) {
        auto file = *(i++);
        if (begin(i) == end(iter))
            t = std::make_shared<task>(task(file.path().string(), true));
        else
            t = std::make_shared<task>(task(file.path().string(), false));
        t->t.start_gen = std::chrono::high_resolution_clock::now();
        {
            std::unique_lock lock(mtx_after_gen);
            after_gen_queue.push(t);
            t->t.pushed_a_gen = std::chrono::high_resolution_clock::now();
        }
        if (t->is_last)
            return;
    }
}

void reader(std::queue<std::shared_ptr<task>> &after_gen_queue,
    std::queue<std::shared_ptr<task>> &after_read_queue) {
    while (true) {
        if (after_gen_queue.empty())
            continue;
        std::shared_ptr<task> t;
        {
            std::unique_lock lock(mtx_after_gen);
            t = after_gen_queue.front();
            after_gen_queue.pop();
            t->t.start_read = std::chrono::high_resolution_clock::now();
        }
        t->html = get_data(t->filename);
        {
            std::unique_lock lock(mtx_after_read);
            after_read_queue.push(t);
            t->t.pushed_a_read = std::chrono::high_resolution_clock::now();
        }
        if (t->is_last)
            return;
    }
}

void extractor(std::queue<std::shared_ptr<task>> &after_read_queue,
    std::queue<std::shared_ptr<task>> &after_extr_queue) {
    while (true) {
        if (after_read_queue.empty())
            continue;
        std::shared_ptr<task> t;
        {
            std::unique_lock lock(mtx_after_read);
            t = after_read_queue.front();
            after_read_queue.pop();
            t->t.start_extr = std::chrono::high_resolution_clock::now();
        }
        t->recipe_ingredients = extract_ingredients(t->html);
        //t->recipe_ingredients = {};
        t->recipe_steps = extract_steps(t->html);
        t->recipe_title = extract_title(t->html);
        t->recipe_img_url = extract_img_url(t->html);
        {
            std::unique_lock lock(mtx_after_extr);
            after_extr_queue.push(t);
            t->t.pushed_a_extr = std::chrono::high_resolution_clock::now();
        }
        if (t->is_last)
            return;
    }
}

void writer(std::queue<std::shared_ptr<task>> &after_extr_queue,
    std::queue<std::shared_ptr<task>> &after_write_queue) {
    while (true) {
        if (after_extr_queue.empty())
            continue;
        std::shared_ptr<task> t;
        {
            std::unique_lock lock(mtx_after_extr);
            t = after_extr_queue.front();
            after_extr_queue.pop();
            t->t.start_write = std::chrono::high_resolution_clock::now();
        }
        //Пишем информацию в БД
        {
            std::unique_lock lock(mtx_after_write);
            after_write_queue.push(t);
            t->t.pushed_a_write = std::chrono::high_resolution_clock::now();
        }
        if (t->is_last)
            return;
    }
}

bool compare(const std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, std::string> &fir,
    const std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, std::string> &sec) {
    return fir.first.time_since_epoch() < sec.first.time_since_epoch();
}

void destructor(std::queue<std::shared_ptr<task>> &after_write_queue,
    time_on_stage &times, std::ofstream &log_file, sqlite3 *db) {
    double time_on_gen = 0.0;
    double time_on_read = 0.0;
    double time_on_extr = 0.0;
    double time_on_write = 0.0;
    double time_on_destr = 0.0;
    double time_wait_a_gen = 0.0;
    double time_wait_a_read = 0.0;
    double time_wait_a_extr = 0.0;
    double time_wait_a_write = 0.0;
    double time_existence = 0.0;
    std::string sql;
    int n_tasks = 0;
    std::vector<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, std::string>> time_points;
    while (true) {
        if (after_write_queue.empty())
            continue;
        std::shared_ptr<task> t;
        {
            std::unique_lock lock(mtx_after_write);
            t = after_write_queue.front();
            after_write_queue.pop();
            t->t.start_destr = std::chrono::high_resolution_clock::now();
        }
        n_tasks++;

        time_on_gen += time_points_to_sec(t->t.start_gen, t->t.pushed_a_gen);
        time_on_read += time_points_to_sec(t->t.start_read, t->t.pushed_a_read);
        time_on_extr += time_points_to_sec(t->t.start_extr, t->t.pushed_a_extr);
        time_on_write += time_points_to_sec(t->t.start_write, t->t.pushed_a_write);
        time_wait_a_gen += time_points_to_sec(t->t.pushed_a_gen, t->t.start_read);
        time_wait_a_read += time_points_to_sec(t->t.pushed_a_read, t->t.start_extr);
        time_wait_a_extr += time_points_to_sec(t->t.pushed_a_extr, t->t.start_write);
        time_wait_a_write += time_points_to_sec(t->t.pushed_a_write, t->t.start_destr);
        time_existence += time_points_to_sec(t->t.start_gen, std::chrono::high_resolution_clock::now());
        std::ostringstream oss;
        oss << "[";
        for (const auto &el: t->recipe_ingredients) {
            oss << "{\"name\": \"";
            oss << el;
            oss << "\"},";
        }
        oss << "]";
        std::string ingredients = oss.str();
        if (ingredients.rfind(',') != std::string::npos)
            ingredients.erase(ingredients.rfind(','), 1);
        oss.str("");
        oss.clear();
        oss << "[";
        for (const auto &el: t->recipe_steps) {
            oss << "{\"name\": \"";
            oss << el;
            oss << "\"},";
        }
        oss << "]";
        std::string steps = oss.str();
        if (steps.rfind(',') != std::string::npos)
            steps.erase(steps.rfind(','), 1);
        oss.clear();
        sql = "INSERT INTO RECIPE VALUES(" + std::to_string(n_tasks) + ", " + std::to_string(t->issue_id) +
            ", " + "'" + t->filename + "'" + ", " + "'" + t->recipe_title + "'" +
                ", " + "'" + ingredients + "'" + ", " + "'" + steps + "'" +
                    ", " + "'" + t->recipe_img_url + "');";
        char *message_error;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &message_error);
        if (rc != SQLITE_OK) {
            std::cout << "SQL error: " << message_error << std::endl;
            sqlite3_free(message_error);
        }
        t->t.end_destr = std::chrono::high_resolution_clock::now();
        time_on_destr += time_points_to_sec(t->t.start_destr, t->t.end_destr);
        time_points.emplace_back(t->t.start_gen, " ID #" + std::to_string(t->id) + " started generation process");
        time_points.emplace_back(t->t.pushed_a_gen, " ID #" + std::to_string(t->id) + " pushed in after-gen queue");
        time_points.emplace_back(t->t.start_read, " ID #" + std::to_string(t->id) + " started reading process");
        time_points.emplace_back(t->t.pushed_a_read, " ID #" + std::to_string(t->id) + " pushed in after-read queue");
        time_points.emplace_back(t->t.start_extr, " ID #" + std::to_string(t->id) + " started extraction process");
        time_points.emplace_back(t->t.pushed_a_extr, " ID #" + std::to_string(t->id) + " pushed in after-extraction queue");
        time_points.emplace_back(t->t.start_write, " ID #" + std::to_string(t->id) + " started writing to DB process");
        time_points.emplace_back(t->t.pushed_a_write, " ID #" + std::to_string(t->id) + " pushed in after-write queue");
        time_points.emplace_back(t->t.start_destr, " ID #" + std::to_string(t->id) + " started destruction process");
        time_points.emplace_back(t->t.end_destr, " ID #" + std::to_string(t->id) + " ended destruction process");
        if (t->is_last)
            break;
    }
    std::sort(time_points.begin(), time_points.end(), compare);
    for (const auto &[time, msg] : time_points) {
        log_file << time << " " << msg << std::endl;
    }
    times = {time_existence / n_tasks, time_on_gen / n_tasks, time_on_read / n_tasks,
        time_on_extr / n_tasks, time_on_write, time_on_destr / n_tasks, time_wait_a_gen / n_tasks,
        time_wait_a_read / n_tasks, time_wait_a_extr / n_tasks, time_wait_a_write / n_tasks};
}

int conveyor_processing(const std::string &target_dir, const std::string& log_file_name,
    const std::string& time_file_name, sqlite3 *db) {
    std::ofstream time_file(time_file_name);
    if (!time_file)
        return -1;
    std::ofstream log_file(log_file_name);
    if (!log_file) {
        time_file.close();
        return -1;
    }
    std::queue<std::shared_ptr<task>> after_gen_queue;
    std::queue<std::shared_ptr<task>> after_read_queue;
    std::queue<std::shared_ptr<task>> after_extr_queue;
    std::queue<std::shared_ptr<task>> after_write_queue;
    std::thread threads[5];
    time_on_stage t = {};
    threads[0] = std::thread(&generator, std::cref(target_dir), std::ref(after_gen_queue));
    threads[1] = std::thread(&reader, std::ref(after_gen_queue), std::ref(after_read_queue));
    threads[2] = std::thread(&extractor, std::ref(after_read_queue), std::ref(after_extr_queue));
    threads[3] = std::thread(&writer, std::ref(after_extr_queue), std::ref(after_write_queue));
    threads[4] = std::thread(&destructor, std::ref(after_write_queue), std::ref(t), std::ref(log_file), db);
    for (auto & thread : threads) {
        thread.join();
    }
    //среднее время существования
    time_file << "Average existence time: " << t.time_existence << " s." << std::endl;
    //среднее время обработки на каждой стадии
    time_file << "Average generation time: " << t.time_on_gen << " s." << std::endl;
    time_file << "Average reading time: " << t.time_on_read << " s." << std::endl;
    time_file << "Average data extraction time: " << t.time_on_extr << " s." << std::endl;
    time_file << "Average data base writing time: " << t.time_on_write << " s." << std::endl;
    time_file << "Average logging and destruction time: " << t.time_on_destr << " s." << std::endl;
    //среднее время ожидания в каждой из очередей
    time_file << "Average waiting in after-generation queue time: " << t.time_wait_a_gen << " s." << std::endl;
    time_file << "Average waiting in after-reading queue time: " << t.time_wait_a_read << " s." << std::endl;
    time_file << "Average waiting in after-extraction queue time: " << t.time_wait_a_extr << " s." << std::endl;
    time_file << "Average waiting in after-writing queue time: " << t.time_wait_a_write << " s." << std::endl;
    time_file.close();
    log_file.close();
    return 0;
}

int main() {
    setlocale(LC_ALL, "RUS");
    //std::string path = "C:/Users/redmi/CLionProjects/AA/cmake-build-debug/recipes/";
    std::string path;
    std::cout << "Enter directory name: " << std::endl;
    std::cin >> path;
    if (!std::filesystem::exists(path)) {
        std::cout << "Directory does not exist" << std::endl;
        return 1;
    }
    std::string log_file_name = "log_file.txt";
    std::string time_file_name = "time_file.txt";
    sqlite3 *db = nullptr;
    int rc = sqlite3_open("recipes.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database" << std::endl;
        return -1;
    }
    //Очистка таблицы
    //Создание новой таблицы, если ее еще нет
    char *message_error;
    std::string creation = "CREATE TABLE IF NOT EXISTS RECIPE("
                            "ID INT PRIMARY KEY NOT NULL, "
                            "ISSUE_ID INT NOT NULL, "
                            "FILENAME TEXT NOT NULL, "
                            "TITLE TEXT NOT NULL, "
                            "INGREDIENTS TEXT NOT NULL, "
                            "STEPS TEXT NOT NULL, "
                            "IMAGE_URL TEXT NOT NULL );";
    rc = sqlite3_exec(db, creation.c_str(), nullptr, nullptr, &message_error);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't create table" << std::endl;
        sqlite3_free(message_error);
        return -1;
    }
    creation = "DELETE FROM RECIPE;";
    rc = sqlite3_exec(db, creation.c_str(), nullptr, nullptr, &message_error);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't truncate table" << std::endl;
        sqlite3_free(message_error);
        return -1;
    }
    conveyor_processing(path, log_file_name, time_file_name, db);
    sqlite3_close(db);
    return 0;
}
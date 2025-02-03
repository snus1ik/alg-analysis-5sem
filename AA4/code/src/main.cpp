#include <iostream>
#include "curl/curl.h"
#include <bits/stdc++.h>
#include <filesystem>
#include <thread>
#include <mutex>
#include "time.h"
#include <unordered_set>

#define TOO_MANY_URLS -1
#define UNABLE_CREATE_DIR -2
#define ERROR_GET_HTML -3
#define MAX_THREADS 64
#define N_PAGES 512
#define N_EXPERIMENTS 1
#define DIRECTORY "recipes"
#define VISITED 1
#define NOT_VISITED 0

std::mutex mtx;

static size_t curl_write(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_html_string(std::string url)
{
    CURL *curl;
    std::string readBuffer;
    CURLcode res;
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if  (res == CURLE_OK)
            return readBuffer;
        std::cout << curl_easy_strerror(res) << std::endl;
        return "";
    }
    std::cout << "error while initing curl" << std::endl;
    return "";
}

std::unordered_set<std::string> get_list_by_regex(const std::string &html_string, const std::regex &reg) {
    std::unordered_set<std::string> result;
    for (std::sregex_iterator it = std::sregex_iterator(html_string.begin(), html_string.end(), reg);
        it != std::sregex_iterator(); it++) {
        std::smatch match;
        match = *it;
        result.insert(match.str(0));
        }
    return result;
}

int sequential_parsing(const std::string &dir_name, const std::string &url, int n_pages_left) {
    int i = 0;
    std::map<std::string, int> pages;
    pages[url] = NOT_VISITED;
    while (n_pages_left > 0) {
        std::string my_url;
        if (!pages.empty()) {
            for (auto &[fst, snd]: pages) {
                if (snd == NOT_VISITED) {
                    snd = VISITED;
                    my_url = fst;
                    break;
                }

            }
        }

        int file_num;
        std::string html_string = get_html_string(my_url);
        if (html_string.empty()) {
            return ERROR_GET_HTML;
        }
        std::regex reg_urls(url + "[A-Za-z0-9-/]*");
        std::unordered_set<std::string> urls = get_list_by_regex(html_string, reg_urls);
        if (html_string.find("Продукты") != std::string::npos) {
            if (n_pages_left <= 0)
                return 0;
            n_pages_left -= 1;
            file_num = n_pages_left;

            std::string file_name = dir_name + "/recipe_" + std::to_string(file_num) + ".html";
            std::ofstream ofs(file_name);
            ofs << html_string;
            ofs.close();
        }

        for (const auto& u : urls) {
            if (!pages.contains(u)) {
                pages[u] = NOT_VISITED;
            }
        }

    }
    return 0;
}

int thread_parser(std::map<std::string, int> &pages, int &n_pages_left,
    const std::string &dir_name, const std::string &url, int id_thread) {
    while (n_pages_left > 0) {
        std::string my_url;
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (n_pages_left <= 0) {
                return 0;
            }
            if (!pages.empty()) {
                for (auto &[fst, snd]: pages) {
                    if (snd == NOT_VISITED) {
                        snd = VISITED;
                        my_url = fst;
                        break;
                    }

                }
            }
        }
        if (my_url.empty())
            continue;

        int file_num;
        std::string html_string = get_html_string(my_url);
        if (html_string.empty()) {
            return ERROR_GET_HTML;
        }
        std::regex reg_urls(url + "[A-Za-z0-9-/]*");
        std::unordered_set<std::string> urls = get_list_by_regex(html_string, reg_urls);
        if (html_string.find("Продукты") != std::string::npos) {

            {
                std::unique_lock<std::mutex> lock(mtx);
                if (n_pages_left <= 0)
                    return 0;
                n_pages_left -= 1;
                file_num = n_pages_left;
            }

            std::string file_name = dir_name + "/recipe_" + std::to_string(file_num) + ".html";
            std::ofstream ofs(file_name);
            ofs << html_string;
            ofs.close();
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            if (n_pages_left <= 0) {
                return 0;
            }
            for (const auto& u : urls) {
                if (!pages.contains(u)) {
                    pages[u] = NOT_VISITED;
                }
            }
        }

    }
    return 0;
}

int parallel_parsing(const std::string &dir_name, const std::string &url, int &n_pages, int &n_threads) {
    int n = n_pages;
    std::map<std::string, int> pages;
    pages[url] = NOT_VISITED;
    std::thread threads[n_threads];
    for (int i = 0; i < n_threads; i++) {
        threads[i] = std::thread(&thread_parser, std::ref(pages), std::ref(n), std::ref(dir_name), std::ref(url), i);
    }
    for (int i = 0; i < n_threads; i++) {
        threads[i].join();
    }
    return 0;
}

int html_from_urls_to_dir(const std::string &dir_name, const std::string &url, int n_pages, int n_threads,
    double &process_time) {
    if (!std::filesystem::exists(dir_name)) {
        if (!std::filesystem::create_directory(dir_name)) {
            std::cout << "Unable to create directory" << std::endl;
            return UNABLE_CREATE_DIR;
        }
    }
    if (n_threads < 1) {
        clock_t begin = clock();
        sequential_parsing(dir_name, url, n_pages);
        process_time = (double) (clock() - begin) / CLOCKS_PER_SEC;
    }
    else {
        clock_t begin = clock();
        parallel_parsing(dir_name, url, n_pages, n_threads);
        process_time = (double) (clock() - begin) / CLOCKS_PER_SEC;
    }
    return 0;
}

int plots(const char *names[], int n_plots, ...) {
    FILE* plot = popen("gnuplot.exe -persist", "w");
    if (plot) {
        va_list args;
        va_start(args, n_plots);
        fprintf(plot, "set encoding utf8\n");
        fprintf(plot, "set xlabel \"Количество потоков\"\n");
        fprintf(plot, "set ylabel \"Страниц/сек\"\n");
        fprintf(plot, "set key left\n");
        fprintf(plot, "plot");
        for (int i = 0; i < n_plots; i++) {
            fprintf(plot, " '%s' with linespoints title \"%s\",", va_arg(args, char*), names[i]);
        }
        fprintf(plot, "\n");
        va_end(args);
        fflush(plot);
        pclose(plot);
        return 0;
    }
    pclose(plot);
    return 1;
}

int main() {
    setlocale(LC_ALL, "RUS");

    int option = 0;
    std::cout << "1) Enter dir name, recipes amount and threads amount, get recipes dir" << std::endl;
    std::cout << "2) Time measures" << std::endl;
    std::cout << "Enter option:" << std::endl;
    std::cin >> option;

    double process_time = 0;
    int n_pages, n_threads;
    std::string url = "https://www.kuhnyatv.ru/recipes/";
    std::string dir_name;

    if (option == 1) {
        std::cout << "Enter directory name: " << std::endl;
        std::cin >> dir_name;
        std::cout << "Enter recipes amount: " << std::endl;
        std::cin >> n_pages;
        std::cout << "Enter threads amount: " << std::endl;
        std::cin >> n_threads;
        std::cout << "Processing..." << std::endl;
        html_from_urls_to_dir(dir_name, url, n_pages, n_threads, process_time);
    }
    else if (option == 2) {
        std::cout << "Processing..." << std::endl;
        const char* file_name = "times.dat";
        double sum_time = 0;
        std::ofstream fp(file_name);
        if (!fp)
            return -1;

        for (int j = 0; j < N_EXPERIMENTS; j++) {
            html_from_urls_to_dir(DIRECTORY, url, N_PAGES, 0, process_time);
            sum_time += process_time / N_PAGES;
        }
        std::cout << 0 << " done" << std::endl;
        std::cout << 0 << " time" << sum_time / N_EXPERIMENTS << std::endl;

        for (int i = 1; i <= MAX_THREADS; i *= 2) {
            sum_time = 0;
            for (int j = 0; j < N_EXPERIMENTS; j++) {
                html_from_urls_to_dir(dir_name, url, N_PAGES, i, process_time);
                sum_time += process_time / N_PAGES;
            }
            std::cout << i << " done" << std::endl;
            std::cout << i << " time " << sum_time / N_EXPERIMENTS << std::endl;
            fp << i << "\t" << sum_time / N_EXPERIMENTS << std::endl;
        }
        fp.close();
        const char *names[] = {"Время получения одного рецепта от количества потоков"};
        plots(names, 1, file_name);
    }
    else
        std::cout << "Invalid option" << std::endl;
    return 0;
}

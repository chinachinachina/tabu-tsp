#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define INF INT_MAX
/* 迭代次数 */
#define ITERATIONS 100
/* 禁忌代数 */
#define TABU_SIZE 10

/* 以下七项为数据集属性 */
/* 数据集名称 */
std::string dataset_name;
/* 数据集类型，此程序只关注TSP类型 */
std::string dataset_type;
/* 数据集附加说明 */
std::string dataset_comment;
/* 数据集维度 */
int dataset_d;
/* 边权类型，此处先研究显式类型（EXPLICIT），非欧几里得二维坐标（EUC_2D） */
std::string dataset_edge_type;
/* 显式数据类型 */
std::string dataset_data_type;
/* 数据组织格式 */
std::string dataset_edge_format;
/* 边权矩阵，可为LOWER_DIAG_ROW或FULL_MATRIX */
std::vector<std::vector<int> > matrix;

/* 官方算例最优解 */ 
std::map<std::string, int> opt_map;
/* 可执行文件的绝对路径 */
std::string abs_path; 
/* 前两列是需要交换的结点，第三列是该次交换的路径长度，第四列是禁忌长度 */
std::vector<std::vector<int> > tabu_list;
/* 随机结点序列 */
std::vector<int> node_seq_1;
std::vector<int> node_seq_2;
/* 最优迭代次数 */
int best_iter;

/* 获取用户输入的文件名并执行算法 */
void run();
/* 禁忌搜索算法 */
double search();
/* 初始化官方算例最优解 */
void optimal_solutions_init();
/* 初始化vector */
void init_vec();
/* 清空vector */
void clear_vec();
/* string去掉首尾空格 */
std::string& trim(std::string &s);

int main(int argc, char *argv[]) {
	/* 获取当前文件绝对路径 */
	abs_path = argv[0];
	/* 查找最后一个反斜杠的位置 */
	for(int i=abs_path.size()-1; i>0; i--) {
		if(abs_path[i] == '\\') {
			/* 除文件名以外的子串的长度 */
			abs_path = abs_path.substr(0, i+1);
			break;
		}
	}
	
	/* 初始化官方算例最优解 */
	optimal_solutions_init();
	/* 获取用户输入的文件名并执行算法 */
	run();
	
	return 0;
}


/* 获取用户输入的文件名并执行算法 */
void run() {
	std::string file_name;
	/* 循环获取用户输入的指定数据文件名执行禁忌搜索算法 */
	while(std::cin >> file_name) {
		dataset_edge_format = "";
		std::string file_path = abs_path + file_name;
		std::ifstream ifs;
		ifs.open(file_path, std::ios::in);
		/* 判断该路径下的文件是否存在，若不存在，则输出错误提示信息 */ 
		if(!ifs) {
			std::cout << "[ERROR] the file " << file_name << " does not exist" << std::endl << std::endl;
		} else {
			/* 解析数据集名称、类型、注释等 */
			std::string s_temp;
			/* 按行读取、解析数据集 */ 
			while(getline(ifs, s_temp)) {
				/* 去除首尾空格 */
				s_temp = trim(s_temp);
				/* stringd定位信息 */
				int loc_temp;
				/* 该行包含有数据集名称信息 */
				if((loc_temp = s_temp.find("NAME")) != std::string::npos) {
					dataset_name = s_temp.substr(loc_temp + 6, s_temp.size() - 6);
				}
				/* 该行包含有边权类型信息，该行包含有TYPE，先放在前面判断 */
				else if((loc_temp = s_temp.find("EDGE_WEIGHT_TYPE")) != std::string::npos) {
					dataset_edge_type = s_temp.substr(loc_temp + 18, s_temp.size() - 18);
					
				}
				/* 该行包含有显式数据类型类型信息 */
				else if((loc_temp = s_temp.find("DISPLAY_DATA_TYPE")) != std::string::npos) {
					dataset_data_type = s_temp.substr(loc_temp + 19, s_temp.size() - 19);
					
				}
				/* 该行包含有数据集类型信息 */
				else if((loc_temp = s_temp.find("TYPE")) != std::string::npos) {
					dataset_type = s_temp.substr(loc_temp + 6, s_temp.size() - 6);
					
				}
				/* 该行包含有数据集附加说明信息 */
				else if((loc_temp = s_temp.find("COMMENT")) != std::string::npos) {
					dataset_comment = s_temp.substr(loc_temp + 9, s_temp.size() - 9);
				}
				/* 该行包含有数据集维度信息 */
				else if((loc_temp = s_temp.find("DIMENSION")) != std::string::npos) {
					dataset_d = atof(s_temp.substr(loc_temp+11, s_temp.size()-11).c_str());
				}
				/* 该行包含有数据组织形式信息 */
				else if((loc_temp = s_temp.find("EDGE_WEIGHT_FORMAT")) != std::string::npos) {
					dataset_edge_format = s_temp.substr(loc_temp + 20, s_temp.size() - 20);
				}
				/* 该行包含有NODE_COORD_SECTION或NODE_COORD_SECTION，即数据读取开端 */
				else if((loc_temp = s_temp.find("SECTION")) != std::string::npos) { 
					/* 此处研究数据集类型为TSP、边权类型为显式的问题 */
					if(dataset_type == "TSP" && dataset_edge_type == "EXPLICIT" && (dataset_edge_format == "FULL_MATRIX")) {
						/* 输出数据集重要信息 */
						std::cout << "[INFO] name: " << dataset_name << std::endl;
						std::cout << "[INFO] type: " << dataset_type << std::endl;
						std::cout << "[INFO] dimension: " << dataset_d << std::endl;
						std::cout << "[INFO] edge_weight_type: " << dataset_edge_type << std::endl;
						/* 读取数据 */
						for(int i=0; i<dataset_d; i++) {
							/* 扩展矩阵二维空间 */ 
							matrix.push_back(std::vector<int>(dataset_d));
							for(int j=0; j<dataset_d; j++) {
								ifs >> matrix[i][j];
							}
						}
						
						/* 生成随机数种子 */
    					srand((unsigned)time(NULL));
    					
					    /* 计时 */
					    clock_t time_start, time_end;
					    time_start = clock();
					    /* 初始化vector */
						init_vec();
					    /* 执行禁忌搜索算法 */
					    double curr_solution = search();
					    time_end = clock();
					    
					    /* 输出信息，依次为此程序此次解、官方最优解、偏差率以及时间 */
					    std::cout << "[INFO] current solution: " << curr_solution << std::endl;
						std::cout << "[INFO] optimal solution: " << opt_map[dataset_name] << std::endl;
						std::cout << "[INFO] deviation rate: " << (curr_solution - opt_map[dataset_name]) * 100 \
							/ opt_map[dataset_name] << "%" << std::endl;
						std::cout << "[INFO] time: " << (time_end - time_start) * 1.0 / CLOCKS_PER_SEC << std::endl;
						
						/* 此时ifs指向数据部分的最后行的末尾 */
						ifs >> s_temp;
						
					} else {
						/* 研究类型不为TSP或边权类型不为SCPLICIT */ 
						std::cout << "[WARN] the 'TYPE' should be 'TSP' and the 'EDGE_WEIGHT_TYPE' should be " << 
							"'EXPLICIT' and the 'EDGE_WEIGHT_FORMAT' should be 'FULL_MATRIX'" << std::endl;
						break; 
					} 
				}
				/* 文件结束标志，退出程序 */
				else if(s_temp.find("EOF") != std::string::npos) {
					/* 清空vector */
					clear_vec();
					break;
				} 
			}
			std::cout << std::endl;
		}
		/* 关闭文件流 */
		ifs.close();
	}
}

/* 生成随机结点序列 */
void generateRandomOrder(std::vector<int> &node_seq){
    for (int i = 0; i < dataset_d; i++){
        node_seq[i] = rand() % dataset_d;
        for (int j = 0; j < i; j++){
        	/* 序列不可有重复元素 */
            if (node_seq[j] == node_seq[i]){
                i--;
                break;
            }
        }
    }
}

/* 计算node_seq序列对应的距离之和 */
double getPathLen(std::vector<int> &node_seq){
	/* 最后一个点到第一个点的距离 */
    double len = matrix[node_seq[dataset_d-1]][node_seq[0]];
    for (int i = 1; i < dataset_d; i++){
        len += matrix[node_seq[i]][node_seq[i-1]];
    }
    return len;
}

/* 更新禁忌序列的禁忌长度 */
void updateTabuList(int len){
    for (int i = 0; i < len; i++)
    	if (tabu_list[i][3] > 0)
            tabu_list[i][3]--;
}

/* 禁忌搜索算法 */
double search(){
	/* 迭代次数 */
    int iter_count;
	/* 邻居大小 */
	int countn;
	/* 可选择的邻居的数量 */
	int neighbour_count = dataset_d * (dataset_d - 1) / 2;
	/* 最短距离 */
    double best_dis;
    /* 全局最优距离 */
	double final_dis = INF, temp_dis;
	
    best_iter = 0;
	
    /* 预生成所有可能的邻域 */
    for (int i = 0, j = 0; j < dataset_d - 1; j++){
        for (int k = j + 1; k < dataset_d; k++){
            tabu_list[i][0] = j;
            tabu_list[i][1] = k;
            tabu_list[i][2] = INF;
            i++;
        }
    }


    /* 生成初始解，迭代60次以跳出局部最优 */
    for (int i = 0; i < 60; i++){
    	/* 生成随机结点序列 */ 
        generateRandomOrder(node_seq_1);
        /* 计算node_seq_1序列对应的距离之和 */
        best_dis = getPathLen(node_seq_1);

		/* 开始求解，迭代次数为INTERATIONS */ 
        iter_count = ITERATIONS;
        int a, b;
        std::vector<int> vec_temp(2, 0);
		/* cur_best分别是邻域号和当前最短距离 */
		std::vector<int> cur_best(2, 0);
        while (iter_count--){
            countn = neighbour_count;
            vec_temp[0] = vec_temp[1] = cur_best[0] = cur_best[1] = INF;
            node_seq_2 = node_seq_1;
            /* 每次迭代搜索的邻域范围为neighbour_count */
            while (countn--){
                /* 交换邻域 */
                a = tabu_list[countn][0];
                b = tabu_list[countn][1];
                std::swap(node_seq_2[a], node_seq_2[b]);
                temp_dis = getPathLen(node_seq_2);
                /* 若新的解在禁忌表中，则只存特赦相关信息 */
                if (tabu_list[countn][3] > 0){ 
                    tabu_list[countn][2] = INF; 
                    if (temp_dis < vec_temp[1]){
                        vec_temp[0] = countn;
                        vec_temp[1] = temp_dis;
                    }
                } else {
                	/* 否则将距离存储 */
                    tabu_list[countn][2] = temp_dis;
                }
                /* 恢复原状，以便后续使用 */
                std::swap(node_seq_2[a], node_seq_2[b]);   
            }
            /* 遍历邻域求得该迭代最佳值 */
            for (int j = 0; j < neighbour_count; j++){
                if (tabu_list[j][3] == 0 && tabu_list[j][2] < cur_best[1]){
                	/* 更新邻域号和当前最短距离 */
                    cur_best[0] = j;
                    cur_best[1] = tabu_list[j][2];
                }
            }
            /* 特赦 */
            if (cur_best[0] == INF || vec_temp[1] < best_dis) {
                cur_best[0] = vec_temp[0];
                cur_best[1] = vec_temp[1];
            }
			/* 更新此代最优best_dis，设置禁忌长度为10，并对node_seq_1序列进行更改 */
            if (cur_best[1] < best_dis){
                best_dis = cur_best[1];
                tabu_list[cur_best[0]][3] = TABU_SIZE;
                best_iter = ITERATIONS - iter_count;
                a = tabu_list[cur_best[0]][0];
                b = tabu_list[cur_best[0]][1];
                std::swap(node_seq_1[a], node_seq_1[b]);
            }
            /* 更新禁忌序列的禁忌长度 */
            updateTabuList(neighbour_count);
        }
        /* 更新全局最优 */
        if (best_dis < final_dis) final_dis = best_dis;
    }
    return final_dis;
}

/* 初始化官方算例最优解 */
void optimal_solutions_init() {
	opt_map.insert({"gr17", 2085});
	opt_map.insert({"gr21", 2707});
	opt_map.insert({"gr24", 1272});
	opt_map.insert({"swiss42", 1273});
}

/* 初始化vector */
void init_vec() {
	for(int i=0; i<2000; i++)
		tabu_list.push_back(std::vector<int>(4, 0));
	for(int i=0; i<dataset_d; i++) {
		node_seq_1.push_back(0);
		node_seq_2.push_back(0);
	}
}

/* 清空vector */
void clear_vec() {
	matrix.clear();
	tabu_list.clear();
	node_seq_1.clear();
	node_seq_2.clear();
}

/* string去掉首尾空格 */
std::string& trim(std::string &s) {
	if(s.empty()) return s;
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
} 

#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define INF INT_MAX
/* �������� */
#define ITERATIONS 100
/* ���ɴ��� */
#define TABU_SIZE 10

/* ��������Ϊ���ݼ����� */
/* ���ݼ����� */
std::string dataset_name;
/* ���ݼ����ͣ��˳���ֻ��עTSP���� */
std::string dataset_type;
/* ���ݼ�����˵�� */
std::string dataset_comment;
/* ���ݼ�ά�� */
int dataset_d;
/* ��Ȩ���ͣ��˴����о���ʽ���ͣ�EXPLICIT������ŷ����ö�ά���꣨EUC_2D�� */
std::string dataset_edge_type;
/* ��ʽ�������� */
std::string dataset_data_type;
/* ������֯��ʽ */
std::string dataset_edge_format;
/* ��Ȩ���󣬿�ΪLOWER_DIAG_ROW��FULL_MATRIX */
std::vector<std::vector<int> > matrix;

/* �ٷ��������Ž� */ 
std::map<std::string, int> opt_map;
/* ��ִ���ļ��ľ���·�� */
std::string abs_path; 
/* ǰ��������Ҫ�����Ľ�㣬�������Ǹôν�����·�����ȣ��������ǽ��ɳ��� */
std::vector<std::vector<int> > tabu_list;
/* ���������� */
std::vector<int> node_seq_1;
std::vector<int> node_seq_2;
/* ���ŵ������� */
int best_iter;

/* ��ȡ�û�������ļ�����ִ���㷨 */
void run();
/* ���������㷨 */
double search();
/* ��ʼ���ٷ��������Ž� */
void optimal_solutions_init();
/* ��ʼ��vector */
void init_vec();
/* ���vector */
void clear_vec();
/* stringȥ����β�ո� */
std::string& trim(std::string &s);

int main(int argc, char *argv[]) {
	/* ��ȡ��ǰ�ļ�����·�� */
	abs_path = argv[0];
	/* �������һ����б�ܵ�λ�� */
	for(int i=abs_path.size()-1; i>0; i--) {
		if(abs_path[i] == '\\') {
			/* ���ļ���������Ӵ��ĳ��� */
			abs_path = abs_path.substr(0, i+1);
			break;
		}
	}
	
	/* ��ʼ���ٷ��������Ž� */
	optimal_solutions_init();
	/* ��ȡ�û�������ļ�����ִ���㷨 */
	run();
	
	return 0;
}


/* ��ȡ�û�������ļ�����ִ���㷨 */
void run() {
	std::string file_name;
	/* ѭ����ȡ�û������ָ�������ļ���ִ�н��������㷨 */
	while(std::cin >> file_name) {
		dataset_edge_format = "";
		std::string file_path = abs_path + file_name;
		std::ifstream ifs;
		ifs.open(file_path, std::ios::in);
		/* �жϸ�·���µ��ļ��Ƿ���ڣ��������ڣ������������ʾ��Ϣ */ 
		if(!ifs) {
			std::cout << "[ERROR] the file " << file_name << " does not exist" << std::endl << std::endl;
		} else {
			/* �������ݼ����ơ����͡�ע�͵� */
			std::string s_temp;
			/* ���ж�ȡ���������ݼ� */ 
			while(getline(ifs, s_temp)) {
				/* ȥ����β�ո� */
				s_temp = trim(s_temp);
				/* stringd��λ��Ϣ */
				int loc_temp;
				/* ���а��������ݼ�������Ϣ */
				if((loc_temp = s_temp.find("NAME")) != std::string::npos) {
					dataset_name = s_temp.substr(loc_temp + 6, s_temp.size() - 6);
				}
				/* ���а����б�Ȩ������Ϣ�����а�����TYPE���ȷ���ǰ���ж� */
				else if((loc_temp = s_temp.find("EDGE_WEIGHT_TYPE")) != std::string::npos) {
					dataset_edge_type = s_temp.substr(loc_temp + 18, s_temp.size() - 18);
					
				}
				/* ���а�������ʽ��������������Ϣ */
				else if((loc_temp = s_temp.find("DISPLAY_DATA_TYPE")) != std::string::npos) {
					dataset_data_type = s_temp.substr(loc_temp + 19, s_temp.size() - 19);
					
				}
				/* ���а��������ݼ�������Ϣ */
				else if((loc_temp = s_temp.find("TYPE")) != std::string::npos) {
					dataset_type = s_temp.substr(loc_temp + 6, s_temp.size() - 6);
					
				}
				/* ���а��������ݼ�����˵����Ϣ */
				else if((loc_temp = s_temp.find("COMMENT")) != std::string::npos) {
					dataset_comment = s_temp.substr(loc_temp + 9, s_temp.size() - 9);
				}
				/* ���а��������ݼ�ά����Ϣ */
				else if((loc_temp = s_temp.find("DIMENSION")) != std::string::npos) {
					dataset_d = atof(s_temp.substr(loc_temp+11, s_temp.size()-11).c_str());
				}
				/* ���а�����������֯��ʽ��Ϣ */
				else if((loc_temp = s_temp.find("EDGE_WEIGHT_FORMAT")) != std::string::npos) {
					dataset_edge_format = s_temp.substr(loc_temp + 20, s_temp.size() - 20);
				}
				/* ���а�����NODE_COORD_SECTION��NODE_COORD_SECTION�������ݶ�ȡ���� */
				else if((loc_temp = s_temp.find("SECTION")) != std::string::npos) { 
					/* �˴��о����ݼ�����ΪTSP����Ȩ����Ϊ��ʽ������ */
					if(dataset_type == "TSP" && dataset_edge_type == "EXPLICIT" && (dataset_edge_format == "FULL_MATRIX")) {
						/* ������ݼ���Ҫ��Ϣ */
						std::cout << "[INFO] name: " << dataset_name << std::endl;
						std::cout << "[INFO] type: " << dataset_type << std::endl;
						std::cout << "[INFO] dimension: " << dataset_d << std::endl;
						std::cout << "[INFO] edge_weight_type: " << dataset_edge_type << std::endl;
						/* ��ȡ���� */
						for(int i=0; i<dataset_d; i++) {
							/* ��չ�����ά�ռ� */ 
							matrix.push_back(std::vector<int>(dataset_d));
							for(int j=0; j<dataset_d; j++) {
								ifs >> matrix[i][j];
							}
						}
						
						/* ������������� */
    					srand((unsigned)time(NULL));
    					
					    /* ��ʱ */
					    clock_t time_start, time_end;
					    time_start = clock();
					    /* ��ʼ��vector */
						init_vec();
					    /* ִ�н��������㷨 */
					    double curr_solution = search();
					    time_end = clock();
					    
					    /* �����Ϣ������Ϊ�˳���˴ν⡢�ٷ����Ž⡢ƫ�����Լ�ʱ�� */
					    std::cout << "[INFO] current solution: " << curr_solution << std::endl;
						std::cout << "[INFO] optimal solution: " << opt_map[dataset_name] << std::endl;
						std::cout << "[INFO] deviation rate: " << (curr_solution - opt_map[dataset_name]) * 100 \
							/ opt_map[dataset_name] << "%" << std::endl;
						std::cout << "[INFO] time: " << (time_end - time_start) * 1.0 / CLOCKS_PER_SEC << std::endl;
						
						/* ��ʱifsָ�����ݲ��ֵ�����е�ĩβ */
						ifs >> s_temp;
						
					} else {
						/* �о����Ͳ�ΪTSP���Ȩ���Ͳ�ΪSCPLICIT */ 
						std::cout << "[WARN] the 'TYPE' should be 'TSP' and the 'EDGE_WEIGHT_TYPE' should be " << 
							"'EXPLICIT' and the 'EDGE_WEIGHT_FORMAT' should be 'FULL_MATRIX'" << std::endl;
						break; 
					} 
				}
				/* �ļ�������־���˳����� */
				else if(s_temp.find("EOF") != std::string::npos) {
					/* ���vector */
					clear_vec();
					break;
				} 
			}
			std::cout << std::endl;
		}
		/* �ر��ļ��� */
		ifs.close();
	}
}

/* �������������� */
void generateRandomOrder(std::vector<int> &node_seq){
    for (int i = 0; i < dataset_d; i++){
        node_seq[i] = rand() % dataset_d;
        for (int j = 0; j < i; j++){
        	/* ���в������ظ�Ԫ�� */
            if (node_seq[j] == node_seq[i]){
                i--;
                break;
            }
        }
    }
}

/* ����node_seq���ж�Ӧ�ľ���֮�� */
double getPathLen(std::vector<int> &node_seq){
	/* ���һ���㵽��һ����ľ��� */
    double len = matrix[node_seq[dataset_d-1]][node_seq[0]];
    for (int i = 1; i < dataset_d; i++){
        len += matrix[node_seq[i]][node_seq[i-1]];
    }
    return len;
}

/* ���½������еĽ��ɳ��� */
void updateTabuList(int len){
    for (int i = 0; i < len; i++)
    	if (tabu_list[i][3] > 0)
            tabu_list[i][3]--;
}

/* ���������㷨 */
double search(){
	/* �������� */
    int iter_count;
	/* �ھӴ�С */
	int countn;
	/* ��ѡ����ھӵ����� */
	int neighbour_count = dataset_d * (dataset_d - 1) / 2;
	/* ��̾��� */
    double best_dis;
    /* ȫ�����ž��� */
	double final_dis = INF, temp_dis;
	
    best_iter = 0;
	
    /* Ԥ�������п��ܵ����� */
    for (int i = 0, j = 0; j < dataset_d - 1; j++){
        for (int k = j + 1; k < dataset_d; k++){
            tabu_list[i][0] = j;
            tabu_list[i][1] = k;
            tabu_list[i][2] = INF;
            i++;
        }
    }


    /* ���ɳ�ʼ�⣬����60���������ֲ����� */
    for (int i = 0; i < 60; i++){
    	/* �������������� */ 
        generateRandomOrder(node_seq_1);
        /* ����node_seq_1���ж�Ӧ�ľ���֮�� */
        best_dis = getPathLen(node_seq_1);

		/* ��ʼ��⣬��������ΪINTERATIONS */ 
        iter_count = ITERATIONS;
        int a, b;
        std::vector<int> vec_temp(2, 0);
		/* cur_best�ֱ�������ź͵�ǰ��̾��� */
		std::vector<int> cur_best(2, 0);
        while (iter_count--){
            countn = neighbour_count;
            vec_temp[0] = vec_temp[1] = cur_best[0] = cur_best[1] = INF;
            node_seq_2 = node_seq_1;
            /* ÿ�ε�������������ΧΪneighbour_count */
            while (countn--){
                /* �������� */
                a = tabu_list[countn][0];
                b = tabu_list[countn][1];
                std::swap(node_seq_2[a], node_seq_2[b]);
                temp_dis = getPathLen(node_seq_2);
                /* ���µĽ��ڽ��ɱ��У���ֻ�����������Ϣ */
                if (tabu_list[countn][3] > 0){ 
                    tabu_list[countn][2] = INF; 
                    if (temp_dis < vec_temp[1]){
                        vec_temp[0] = countn;
                        vec_temp[1] = temp_dis;
                    }
                } else {
                	/* ���򽫾���洢 */
                    tabu_list[countn][2] = temp_dis;
                }
                /* �ָ�ԭ״���Ա����ʹ�� */
                std::swap(node_seq_2[a], node_seq_2[b]);   
            }
            /* ����������øõ������ֵ */
            for (int j = 0; j < neighbour_count; j++){
                if (tabu_list[j][3] == 0 && tabu_list[j][2] < cur_best[1]){
                	/* ��������ź͵�ǰ��̾��� */
                    cur_best[0] = j;
                    cur_best[1] = tabu_list[j][2];
                }
            }
            /* ���� */
            if (cur_best[0] == INF || vec_temp[1] < best_dis) {
                cur_best[0] = vec_temp[0];
                cur_best[1] = vec_temp[1];
            }
			/* ���´˴�����best_dis�����ý��ɳ���Ϊ10������node_seq_1���н��и��� */
            if (cur_best[1] < best_dis){
                best_dis = cur_best[1];
                tabu_list[cur_best[0]][3] = TABU_SIZE;
                best_iter = ITERATIONS - iter_count;
                a = tabu_list[cur_best[0]][0];
                b = tabu_list[cur_best[0]][1];
                std::swap(node_seq_1[a], node_seq_1[b]);
            }
            /* ���½������еĽ��ɳ��� */
            updateTabuList(neighbour_count);
        }
        /* ����ȫ������ */
        if (best_dis < final_dis) final_dis = best_dis;
    }
    return final_dis;
}

/* ��ʼ���ٷ��������Ž� */
void optimal_solutions_init() {
	opt_map.insert({"gr17", 2085});
	opt_map.insert({"gr21", 2707});
	opt_map.insert({"gr24", 1272});
	opt_map.insert({"swiss42", 1273});
}

/* ��ʼ��vector */
void init_vec() {
	for(int i=0; i<2000; i++)
		tabu_list.push_back(std::vector<int>(4, 0));
	for(int i=0; i<dataset_d; i++) {
		node_seq_1.push_back(0);
		node_seq_2.push_back(0);
	}
}

/* ���vector */
void clear_vec() {
	matrix.clear();
	tabu_list.clear();
	node_seq_1.clear();
	node_seq_2.clear();
}

/* stringȥ����β�ո� */
std::string& trim(std::string &s) {
	if(s.empty()) return s;
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
} 

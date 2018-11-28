/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definições relativas a plataforma e tipos de dados 
	 * utilizados para representar números inteiros e de ponto flutuante.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_DOGMAS_INCLUIDOS
#define SIMULADOR_AUXILIAR_DOGMAS_INCLUIDOS sim

#include <string> //strings de texto
#include <array>  //vetor de dimensão fixa
#include <vector> //vetor de dimensão variável
#include <cmath>  //funções matemáticas em implementação padrão
#include <stdexcept> //exceções para captura de erros
#include <memory> //unique_ptr, para manipular destrutor de alocação dinâmica
#include <algorithm> //algorítmos padrão (eg.: find)
#include <map> //mapa entre dois tipos de valores, com chave única
#include <cstdint> //definição padrão de tipos inteitos com tamanho
#include <utility> //templates utilitários (eg.: std::pair)
#include <functional> //templates para funções genéricas
#include <set> //conjunto de valores únicos ordenados
#include <unordered_set> //conjunto de valores únicos
#include <deque> //pilha bidirecional

#include <iostream> //saída para console
#include <iomanip> //manipulação de streams de entrada/saída

//o tipo usado para indexação de elementos é um inteiro de 32 bits
typedef std::int32_t          ind_t;
typedef std::array<ind_t, 2> ind2_t;
typedef std::vector<ind_t>   inds_t;

typedef double NUM_T;
typedef std::vector<NUM_T> NUMs_t;

#endif //fim header guard SIMULADOR_AUXILIAR_DOGMAS_INCLUIDOS
/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definições de classe que representa qualquer objeto conectado a um conjunto de nodos.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_CONECTADO_INCLUIDO
#define SIMULADOR_AUXILIAR_CONECTADO_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

template <typename T>
struct Conectado
{
	T      elemento;
	inds_t conexoes;
	ind_t  conexoes_N()const{return conexoes.size();}
	Conectado(T Elemento, inds_t Conexoes)
		: elemento(std::move(Elemento)), conexoes(std::move(Conexoes)){}
	template <class ... CONEXOES_TS>
	Conectado(T Elemento, CONEXOES_TS ... Conexoes)
		: elemento(std::move(Elemento)), conexoes{static_cast<ind_t>(Conexoes)...}{}
	//construtores padrão
		Conectado() = default;
};

#endif //fim header guard SIMULADOR_AUXILIAR_CONECTADO_INCLUIDO
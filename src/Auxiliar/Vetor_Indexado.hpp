/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de um vetor que atribui índice fixo aos objetos que contém.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_VETOR_INDEXADO_INCLUIDO
#define SIMULADOR_AUXILIAR_VETOR_INDEXADO_INCLUIDO sim

#include <Auxiliar/Indexador.hpp>

//Classe que encapsula vetor de elementos com índice fixado
template<typename T>
class VetorIndexado
{
	Indexador     indexador; //gerador de índices
	std::vector<T>      mem; //vetor de itens na memória
public:
	VetorIndexado()=default;
	//operadores de acesso (não verificam validade do índice)
	T       & operator[](std::size_t k)     {return mem[k];}
	T const & operator[](std::size_t k)const{return mem[k];}
	//adiciona item ao vetor e retorna seu índice fixo
	ind_t Adiciona(T v)
	{
		ind_t i = indexador.Cria();
		while( i >= static_cast<ind_t>(mem.size())) //se requerido, expande memória
			{mem.resize(std::max(std::size_t(1), std::size_t(mem.size()*2)));}
		mem[i] = std::move(v);
		return i;
	}
	//Remove item de índice [i]. [i] deve ter sido previamente retornado por Adiciona(...) e pode ser retornado para futuros itens adicionados após a remoção
	void Remove(ind_t i) //comportamento indefinido se i não é retorno prévio de Adiciona(T) ou já removido
	{
		indexador.Libera(i);
	}
	ind_t Quantidade()const
	{
		return indexador.Contagem();
	}
	//retorna acesso de leitura a vetor de índices utilizados em ordem crescente
	inds_t const & ContiguoAcesso()const
	{
		return indexador.ContiguoAcesso();
	}
};

#endif //fim header guard SIMULADOR_AUXILIAR_VETOR_INDEXADO_INCLUIDO
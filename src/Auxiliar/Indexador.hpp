/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém estrutura que gera índices únicos ordenados, reaproveitando índices liberados antes de expandir.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_AUXILIAR_INDEXADOR_INCLUIDO
#define SIMULADOR_AUXILIAR_INDEXADOR_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

//Classe que representa gerador de índices únicos
class Indexador
{
	ind_t contador;
	std::deque<ind_t> livres;
	inds_t mutable contiguo;
	bool   mutable contiguo_relaxado;
public:
	Indexador():contador(0),contiguo_relaxado(false){;}
	//retorna índice que será considerado reservado até ser liberado
	ind_t Cria()
	{
		contiguo_relaxado = true;
		if(livres.empty())
			{
				ind_t i = contador;
				contador++;
				return i;
			}
		else
			{
				ind_t i = livres.front();
				livres.pop_front();
				return i;
			}
	};
	//libera índice [i] para reutilização
	//// indefinido se [i] fornecido não-previamente retornado por Cria()
	//// indefinido se [i] for liberado mais de uma vez
	void  Libera(ind_t i) 
	{
		contiguo_relaxado = true;
		livres.emplace_back(i);
	}
	//retorna quantos índices estão atualmente reservados
	ind_t Contagem()const
	{
		//o número de itens em uso é a contagem máxima subtraida da quantidade de itens liberados
		return(contador - static_cast<ind_t>(livres.size()));
	}
	//retorna acesso de leitura a vetor de índices utilizados em ordem crescente
	inds_t const & ContiguoAcesso()const
	{
		if(contiguo_relaxado)
		{
			ind_t const tot = contador-livres.size();
			//buffer temporário ordenado de índices livres
				std::vector<ind_t> buff;
				buff.reserve(livres.size());
				for(auto const & il : livres)
					{buff.emplace_back(il);}
				std::sort(buff.begin(), buff.end());
			//constrói vetor contiguo
				contiguo.clear();
				contiguo.reserve(tot);
				auto pulo_it = buff.begin();
				ind_t   pulo = buff.empty() ? -1 : *pulo_it;
				for(ind_t i = 0; i<contador; i++)
				{
					if(pulo == i) //se [i] estava na lista de livres, ignora-o
					{
						if((++pulo_it)!=buff.end())
							{pulo = *pulo_it;}
					}
					else //se [i] é utilizado, salva no vetor de índices contíguos
						{contiguo.emplace_back(i);}
				}
			//sinaliza que [contiguo] reflete estado atual
			contiguo_relaxado = false;
		}
		return contiguo;
	}
};

#endif //fim header guard SIMULADOR_AUXILIAR_INDEXADOR_INCLUIDO
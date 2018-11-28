/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de um grafo orientado,
	 * com operações requeridas para:
	 * 		a) aplicar restrições topológicas ao circuito
	 * 		b) mapear nodos de circuito para equacionamento
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_GRAFO_INCLUIDO
#define SIMULADOR_CONDICIONAMENTO_GRAFO_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Condicionamento {

namespace Topologia
{
	
struct Aresta
{
	ind_t v0;
	ind_t v1;
};

struct Vertice
{
	inds_t as;	
};

struct Grafo
{
	std::vector<Vertice> verts;
	std::vector<Aresta> arests;
	Grafo() = default;
	explicit Grafo(ind_t N_vertices) : verts(N_vertices) {};
	ind_t ContaArestas()const
	{
		return arests.size();
	}
	ind_t ContaVertices()const
	{
		return verts.size();
	}
	//insere Aresta direcionada de v0 a v1
	ind_t AdicionaAresta(ind_t v0, ind_t v1)
	{
		//índice do aresta sendo criado é o tamanho atual
		ind_t ai = arests.size();
			verts[v0].as.push_back(ai);
			verts[v1].as.push_back(ai);
			arests.emplace_back(Aresta{v0, v1});
		return ai;
	}
	//retorna se todo nodo é referenciado por, pelo menos, 2 ramos (com índice de contra-exemplo, se houver)
	// a junção desta garantia com Nenhum Ramo Singular que permite inferir caminhos fechados
	std::pair<bool, ind_t> CaminhosFechados()const
	{
		if(verts.empty())
			{return std::pair<bool, ind_t>{false, 0};}
		//vértices requerem 2 conexões
			for(ind_t k = 0; k < static_cast<ind_t>(verts.size()); k++)
			{
				if(verts[k].as.size()<2)
					{return std::pair<bool, ind_t>{false, k};}
			}
		return std::pair<bool, ind_t>{true, 0};
	}
	//retorna se não existe ramo que conecta-se ao mesmo nodo nas duas extremidades (junto a íncide de contra-exemplo, se houver)
	std::pair<bool, ind_t> NenhumRamoSingular()const
	{
		ind_t const m = arests.size();
		for(ind_t k=0; k<m; k++)
		{
			if(arests[k].v0 == arests[k].v1)
				{return std::pair<bool, ind_t>{false, k};}
		}
		return std::pair<bool, ind_t>{true, 0};
	}
	//retorna se todos os vértices são conectados
	bool TodosNodosConectados()const
	{
		//pré condições
			ind_t N  = verts.size();
			if(N < 2)
			{
				return false;
			}
			if(arests.empty())
			{
				return false;
			}
			std::set<ind_t>    alvo;
			std::set<ind_t> ineditos;
		//manualmente adiciona pivô
			ind_t const pivo = arests[0].v0;
			alvo.emplace(pivo);
			ineditos.emplace(pivo);
		//etapa 1, até metade dos nodos serem encontrados
			ind_t const Nb = N/2;		
			while( (!ineditos.empty()) && (static_cast<ind_t>(alvo.size()) < Nb) )
			{
				ineditos = nao_no_alvo(ineditos, alvo);
			};
			if(ineditos.empty())
			{
				return (static_cast<ind_t>(alvo.size())==N);
			}
		//etapa 2, metade final dos nodos
			//converte alvo de "já atingidos" para "ainda não atingidos"
			alvo = converte_para_faltantes(alvo, N);
			while( !ineditos.empty() )
			{
				ineditos = no_alvo(ineditos, alvo);
			};
		return alvo.empty();
	}
	ind_t VerticeGrau(ind_t j)const
	{
		return static_cast<ind_t>(verts[j].as.size());
	}
protected:
	//função auxiliar
	std::set<ind_t> no_alvo(std::set<ind_t> const & origem, std::set<ind_t> & alvo)const
	{
		std::set<ind_t> ineditos;
		for(auto const & vo : origem)
		{
			Vertice const & v = verts[vo];
			//procura por todas as arestas do vértive
			for(auto const & ai : v.as)
			{
				Aresta const & a = arests[ai];
				ind_t o = (vo==a.v0)? a.v1 : a.v0; //seleciona "outro vértice" da aresta
				auto procura = alvo.lower_bound(o);
				if(procura != alvo.end())
				{
					if((*procura) == o)
					{
						alvo.erase(procura);
						ineditos.emplace(o);
					}
				}
				continue;
			}
		}		
		return ineditos;
	}
	std::set<ind_t> nao_no_alvo(std::set<ind_t> const & origem, std::set<ind_t> & alvo)const
	{
		std::set<ind_t> ineditos;
		for(auto const & vo : origem)
		{
			Vertice const & v = verts[vo];
			//procura por todas as arestas do vértive
			for(auto const & ai : v.as)
			{
				Aresta const & a = arests[ai];
				ind_t o = (vo==a.v0)? a.v1 : a.v0; //seleciona "outro vértice" da aresta
				auto procura = alvo.lower_bound(o);
				if(procura != alvo.end())
				{
					if((*procura) == o)
					{
						continue;
					}
				}
				alvo.emplace_hint(procura, o);
				ineditos.emplace(o);
			}
		}
		return ineditos;
	}
	//função auxiliar interna que converte conjunto de vértices para complementar do total
		static std::set<ind_t> converte_para_faltantes(std::set<ind_t> const & V, ind_t const N)
		{
			std::set<ind_t> faltantes;
			ind_t i = 0;
			auto vi = V.begin();
			auto vf = V.end();
			ind_t a;
			while(i<N)
			{
				if(vi != vf)
				{
					a = *vi;
					vi++;
				}
				else
				{
					a = N;
				}
				while(i<a)
				{
					faltantes.emplace(i);
					i++;
				}
				i++;
			}
			return faltantes;
		} //fim de funções auxiliares
};//fim de classe Grafo

} //fim namespace Topologia


} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_GRAFO_INCLUIDO
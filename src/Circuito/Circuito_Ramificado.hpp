 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Circuito composto por ramos básicos
	 * 
	Mateus Kahler
*/

#ifndef CIRCUITO_CIRCUITO_RAMIFICADO_INCLUSO
#define CIRCUITO_CIRCUITO_RAMIFICADO_INCLUSO sim

#include <Circuito/Ramos_Base.hpp>
#include <Auxiliar/Vetor_Indexado.hpp>
#include <Auxiliar/Conectado.hpp>

//especialização de Conectado<T> para ramos básicos (assegurado de ter duas conexões)
template <>
struct Conectado<Circuito::Ramo_Base>
{
	Circuito::Ramo_Base elemento;
	std::array<ind_t, 2> conexoes;
	ind_t  conexoes_N()const{return 2;}
	Conectado(Circuito::Ramo_Base Elemento, inds_t Conexoes)
		: elemento(std::move(Elemento)), conexoes{Conexoes[0], Conexoes[1]}{}
	template <class ... CONEXOES_TS>
	Conectado(Circuito::Ramo_Base Elemento, CONEXOES_TS ... Conexoes)
		: elemento(std::move(Elemento)), conexoes{static_cast<ind_t>(Conexoes)...}{}
	//construtor padrão
		Conectado<Circuito::Ramo_Base>() = default;
};

namespace Circuito{

struct Circuito_Ramificado
{
	VetorIndexado<Conectado<Ramo_Base>> ramos; //ramos que compõe o circuito e nodos a que se conectam
	bool   mutable nodo_relaxado;  //sinalizador para avaliação preguiçosa de [nodos_indexados]
	inds_t mutable nodos_indexados;//conjunto ordenado de nodos utilizados
/////////////////
	Circuito_Ramificado()	: nodo_relaxado(true){;}
	ind_t Adiciona(Ramo_Base ramo, ind_t nodo_p, ind_t nodo_n)
	{
		nodo_relaxado = true;
		return ramos.Adiciona(Conectado<Ramo_Base>{std::move(ramo), nodo_p, nodo_n});
	}
	void RemoveRamo(ind_t i)
	{
		nodo_relaxado = true;
		ramos.Remove(i);
	}
	inds_t RamosIndexados()const
	{
		return ramos.ContiguoAcesso();
	}
	inds_t const & NodosIndexados()const
	{
		atualiza_nodos();
		return nodos_indexados;
	}
	void DefineControleCorrente(ind_t ramo_controlado, ind_t ramo_referencia)
	{
		ramos[ramo_controlado].elemento.cp = ramo_referencia;
	}
	void DefineControleDDP(ind_t ramo_controlado, ind_t nodo_referencia_p, ind_t nodo_referencia_n)
	{
		ramos[ramo_controlado].elemento.cp = nodo_referencia_p;
		ramos[ramo_controlado].elemento.cn = nodo_referencia_n;
	}
	Conectado<Ramo_Base> const & RamoConectadoLeitura(ind_t i)const
	{
		return ramos[i];
	}
	//substitui ramo, mantendo conexões
	void SubstituiRamo(ind_t i, Ramo_Base R)
	{
		ramos[i].elemento = R;
	}
private:
	//função que realiza avaliação preguiçosa de nodos utilizados
	void atualiza_nodos()const
	{
		if(nodo_relaxado)
		{
			//percorre todos ramos coletando nodos utilizados
				auto const & ris = RamosIndexados();
				std::unordered_set<ind_t> conjunto_nodos;
				for(auto const & ri : ris)
				{
					auto const & r = ramos[ri];
					conjunto_nodos.insert(r.conexoes[0]);
					conjunto_nodos.insert(r.conexoes[1]);
				}
			//constrói vetor de nodos utilizados e o ordena
				nodos_indexados.clear();
				nodos_indexados.reserve(conjunto_nodos.size());
				for(auto const & ni : conjunto_nodos)
					{nodos_indexados.emplace_back(ni);}
				std::sort(nodos_indexados.begin(), nodos_indexados.end());
			//sinaliza que nodos_indexados reflete o estado atual do circuito
			nodo_relaxado = false;
		}
	}
};

} //fim namespace Circuito

namespace Condicionamento
{
	struct Estado_Subcircuito
	{
		NUMs_t potenciais_conexoes;
		std::vector<Circuito::Estado_Ramo> e_ramos;
	};
} //fim namespace Condicionamento

#endif //fim header guard CIRCUITO_CIRCUITO_RAMIFICADO_INCLUSO
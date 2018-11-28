 /**
	UFRGS - Engenharia Elétrica
	 * 
	 * Circuito composto por subcircuitos
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CIRCUITO_CIRCUITO_DE_SUBCIRCUITOS_INCLUSO
#define SIMULADOR_CIRCUITO_CIRCUITO_DE_SUBCIRCUITOS_INCLUSO sim

#include <Circuito/Circuito_Ramificado.hpp>

namespace Circuito {

class Circuito_de_Subcircuitos
{
	protected:
	//estrutura interna que guarda associação entre subcircuitos adicionados e ramos pertencentes a este
	struct circ_refs_t
	{
		inds_t ramosi; //índices de ramos em [circuito] que compõe o subcircuito
		inds_t nodose; //nodos que foram utilizados como conexão externa
		inds_t nodosi; //nodos que foram utilizados como conexão interna
	};
	std::map<ind_t, ind_t>    mapa_nodosE; //mapa entre nodos externos, fornecidos como conexões de subcircuitos, e nodos internos
	Indexador                 nodos_index; //gerador de índices internos para os nodos
	VetorIndexado<circ_refs_t>   subcircs; //vetor de acesso a associação de cada subciruito adicionado
	Circuito_Ramificado          circuito; //circuito completo atual
public:
	Circuito_de_Subcircuitos()
	{
		mapa_nodosE[0] = nodos_index.Cria(); //reserva índice interno zero para nodo externo zero
	}
	//adiciona subcircuito e retorna índice
	ind_t Adiciona(Circuito_Ramificado const & subcircuito, inds_t const & conexoes)
	{
		auto const & mns = subcircuito.NodosIndexados(); //nodos utilizados pelo subcircuito
		auto const & mrs = subcircuito.RamosIndexados(); //ramos utilizados pelo subcircuito
		ind_t const  mne = conexoes.size(); //número de nodos expostos do subcircuito
		ind_t const  mnt = mns.size();      //número total de nodos do subcircuito
		ind_t const  mrt = mrs.size();      //número total de ramos do subcircuito
		circ_refs_t buff; //buffer de informações da associação do subcircuito sendo inserido
			buff.nodose.resize(mne);
			buff.nodosi.resize(mnt-mne);
			buff.ramosi.resize(mrt);
		//mapeia nodos do subcircuito fornecido para o circuito completo interno
			std::map<ind_t, ind_t> mapa_nodos_mod_int; 
			for(ind_t k = 0; k < mnt; k++)
			{
				if(k<mne) 	//nodos externos podem ser reutilizados
					{
						ind_t i = UsaOuCria(conexoes[k]);
						mapa_nodos_mod_int[mns[k]] = i;
						buff.nodose[k] = i;
					}
				else //nodos internos são sempre inéditos (não são reaproveitáveis fora do subcircuito)
					{
						ind_t i = nodos_index.Cria();
						mapa_nodos_mod_int[mns[k]] = i;
						buff.nodosi[k-mne] = i;
					}
			}
		//internaliza ramos, usando novos (índices internos dos) nodos
			for(ind_t k = 0; k < mrt; k++)
			{
				Conectado<Ramo_Base> const & r = subcircuito.RamoConectadoLeitura(mrs[k]);
				ind_t np = mapa_nodos_mod_int[r.conexoes[0]];
				ind_t nn = mapa_nodos_mod_int[r.conexoes[1]];
				buff.ramosi[k] = circuito.Adiciona(r.elemento, np, nn);
			}
		//internaliza associação completa e retorna índice do subcircuito
		return subcircs.Adiciona(buff);
	}
	//Remove subcircuito de índice [i]
	void Remove(ind_t i)
	{
		circ_refs_t const & cr = subcircs[i]; //informação sobre subcircuito de índice [i]
		for(auto ri : cr.ramosi) //remove ramos internos
			{circuito.RemoveRamo(ri);}
		for(auto nii : cr.nodosi) //remove nodos exclusivamente internos
			{nodos_index.Libera(nii);}
		subcircs.Remove(i); //remove subcircuito
	}
	//retorna acesso ao circuito completo
	Circuito_Ramificado & CircuitoAcesso()
	{
		return circuito;
	}
	//retorna acesso de leitura ao circuito completo
	Circuito_Ramificado const & CircuitoAcesso()const
	{
		return circuito;
	}
	//Retorna índices internos dos ramos do subcircuito [i]
	inds_t const & RamosiDeSubcircuito(ind_t i)const
	{
		return subcircs[i].ramosi;
	}
	//Retorna índices internos dos ramos do subcircuito [i] que contém estado (capacitivos ou indutivos)
	inds_t RamosiDeSubcircuito_comEstado(ind_t i)const
	{
		inds_t r_is;
		for(auto const & r_i : subcircs[i].ramosi)
		{
			if(circuito.ramos[r_i].elemento.ordem1())
				{ r_is.emplace_back(r_i); }
		}
		return r_is;
	}
	inds_t const & ConexoesiDeSubcircuito(ind_t i)const
	{
		return subcircs[i].nodose;
	}
	void DefineControleCorrente(ind2_t comp_ramoi_controlado, ind2_t comp_ramoi_referencia)
	{
		ind_t ramo_controlado = subcircs[comp_ramoi_controlado[0]].ramosi[comp_ramoi_controlado[1]];
		ind_t ramo_referencia = subcircs[comp_ramoi_referencia[0]].ramosi[comp_ramoi_referencia[1]];
		circuito.DefineControleCorrente(ramo_controlado, ramo_referencia);
	}
	void DefineControleDDP(ind2_t comp_ramoi_controlado, ind_t nodo_externo_referencia_p, ind_t nodo_externo_referencia_n)
	{
		ind_t ramo_controlado = subcircs[comp_ramoi_controlado[0]].ramosi[comp_ramoi_controlado[1]];
		ind_t nodo_referencia_p = mapa_nodosE[nodo_externo_referencia_p];
		ind_t nodo_referencia_n = mapa_nodosE[nodo_externo_referencia_n];
		circuito.DefineControleDDP(ramo_controlado, nodo_referencia_p, nodo_referencia_n);
	}
private:
	//função interna que reutiliza índice para conexão externa já existente, ou cria novo se inexistente.
	ind_t UsaOuCria(ind_t k)
	{
		//procura se nodo já foi mapeado
		auto encontra = mapa_nodosE.find(k);
		//se nodo ainda não mapeado, cria nova associação e retorna
		if(encontra==mapa_nodosE.end())
		{
			ind_t i = nodos_index.Cria();
			mapa_nodosE[k] = i;
			return i;
		}
		else //se nodo já mapeado, retorna índice associado
		{
			return encontra->second;
		}
	}
}; //fim classe Circuito_de_Subcircuitos

}//fim namespace Circuito


#endif // fim header guard SIMULADOR_CIRCUITO_CIRCUITO_DE_SUBCIRCUITOS_INCLUSO
/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de circuito esquemático.
	 * Um Circuito Esquemático guarda conexões e identificação de componenentes.
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_CIRCUITO_ESQUEMATICO_INCLUIDO
#define SIMULADOR_CONDICIONAMENTO_CIRCUITO_ESQUEMATICO_INCLUIDO sim

#include <Condicionamento/Componente_Esquematico.hpp>

namespace Condicionamento {

struct Circuito_Esquematico
{
	std::string Nome;
	std::vector<Componente_Esquematico> Componentes;
	std::vector<std::vector<std::string>> Conexoes; //conexões externas seguidas de conexões de controle
	std::vector<Condicao_Inicial_Esquematico> Condicoes_Iniciais;
	ind_t Adiciona(Componente_Esquematico componente, std::vector<std::string> conexoes)
	{
		ind_t i = static_cast<ind_t>(Componentes.size());
		Componentes.emplace_back(std::move(componente));
		Conexoes.emplace_back(conexoes);
		return i;
	}
	template<typename ...CXs>
		ind_t Adiciona(Componente_Esquematico componente, CXs... conexoes)
		{
			ind_t i = static_cast<ind_t>(Componentes.size());
			Componentes.emplace_back(std::move(componente));
			Conexoes.emplace_back(std::vector<std::string>{static_cast<std::string>(conexoes)...});
			return i;
		}
};

} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_CIRCUITO_ESQUEMATICO_INCLUIDO
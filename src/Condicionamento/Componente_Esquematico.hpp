/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo contém definição de componente de esquemático
	 * Os componentes do esquemático são um conjunto de informações que permitem identificar
	 * um componente de forma humanamente legível, como "Resistor" e "Capacitor".
	 * 
	Mateus Kahler
*/

#ifndef SIMULADOR_CONDICIONAMENTO_COMPONENTE_ESQUEMATICO_INCLUIDO
#define SIMULADOR_CONDICIONAMENTO_COMPONENTE_ESQUEMATICO_INCLUIDO sim

#include <Auxiliar/Dogmas.hpp>

namespace Condicionamento {

struct Componente_Esquematico
{
	std::string tipo;
	std::string modelo;
	std::string nome;
	NUMs_t parametros;
	Componente_Esquematico(std::string tipo, std::string modelo, std::string nome)
		: tipo(tipo), modelo(modelo), nome(nome) {;}
	Componente_Esquematico(std::string tipo, std::string modelo, std::string nome, NUMs_t parametros)
		: tipo(tipo), modelo(modelo), nome(nome), parametros(parametros) {;}
	template<typename ...CXs>
	Componente_Esquematico(std::string tipo, std::string modelo, std::string nome, CXs... params)
		: tipo(tipo), modelo(modelo), nome(nome), parametros{static_cast<NUM_T>(params)...} {;}
};

struct Condicao_Inicial_Esquematico
{
	std::string nome_comp;
	NUM_T valor;
	Condicao_Inicial_Esquematico(std::string componente_nome, NUM_T condicao_valor)
		: nome_comp(componente_nome), valor(condicao_valor) {;}
};

} //fim namespace Condicionamento

#endif //fim header guard SIMULADOR_CONDICIONAMENTO_COMPONENTE_ESQUEMATICO_INCLUIDO
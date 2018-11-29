/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Este arquivo gera circuitos que testam o algorítmo de substituição
	 * de elementos passivos sem estado inicial definido por equivalente 
	 * de condição quiescente (derivada nula).
	 * 
	Mateus Kahler
*/

#include <Condicionamento/Circuito_Esquematico.hpp>
#include <Condicionamento/Analise_Quiescente.hpp>

auto const estoque = Estoque::Estoque_t::Estoque_Pre_Compilado();
	
void teste_autoCI_Capacitor()
{
	//monta esquemático
		Condicionamento::Circuito_Esquematico Esquema;
		Condicionamento::Componente_Esquematico I1("FonteIfixa", "ideal", "I1", 10);
		Condicionamento::Componente_Esquematico R1("Resistor", "ideal", "R1", 3);
		Condicionamento::Componente_Esquematico R2("Resistor", "ideal", "R2", 2);
		Condicionamento::Componente_Esquematico C1("Capacitor", "ideal", "C1", 2);
		Esquema.Adiciona(I1, "0", "n1");
		Esquema.Adiciona(R1, "n1", "n2");
		Esquema.Adiciona(R2, "n2", "0");
		Esquema.Adiciona(C1, "n1", "n2");
		
	//monta análise e objetivos desta
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("C1", "u");
		
		analise_Q1.Adiciona_Objetivo("n1");
		analise_Q1.Adiciona_Objetivo("n2");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
	
	//imprime resultados
		std::cout << resultado << std::endl;
};

void teste_autoCI_Indutor()
{
	//monta esquemático
		Condicionamento::Circuito_Esquematico Esquema;
		Condicionamento::Componente_Esquematico I1("FonteIfixa", "ideal", "I1", 10);
		Condicionamento::Componente_Esquematico R1("Resistor", "ideal", "R1", 3);
		Condicionamento::Componente_Esquematico R2("Resistor", "ideal", "R2", 2);
		Condicionamento::Componente_Esquematico L1("Indutor", "ideal", "L1", 2);
		Esquema.Adiciona(I1, "0", "n1");
		Esquema.Adiciona(R1, "n1", "n2");
		Esquema.Adiciona(R2, "n2", "0");
		Esquema.Adiciona(L1, "n1", "n2");
		
	//monta análise e objetivos desta
		Condicionamento::Analise_Quiescente analise_Q1;
		analise_Q1.Adiciona_Objetivo("L1", "i");
		
		analise_Q1.Adiciona_Objetivo("n1");
		analise_Q1.Adiciona_Objetivo("n2");
		
	//resolve circuito
		auto resultado = analise_Q1.resolve(Esquema, estoque);
	
	//imprime resultados
		std::cout << resultado << std::endl;
};

int main()
{
	std::cout << "Teste com capacitor: " << std::endl;
	teste_autoCI_Capacitor();
	std::cout << "Teste com indutor: " << std::endl;
	teste_autoCI_Indutor();
	return 0;
}
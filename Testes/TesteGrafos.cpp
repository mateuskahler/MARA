/**
	UFRGS - Engenharia Elétrica
	 * 
	 * Sendo que o Grafo contém 3 propriedades que estão classificadas de forma binária,
	 * é possível identenficar oito combinações destas propriedades. Este arquivo gera 
	 * um grafo dentro de cada uma destas oito e testa o sistema para todos.
	 * 
	Mateus Kahler
*/

#include <Condicionamento/Grafo.hpp>
	
int main(int argc, char ** argv)
{
	using namespace std;
	using Condicionamento::Topologia::Grafo;
	cout << "    singular?  fechado?   conexo?  \n";
	auto imprime_testes = [](Grafo & G)->void
	{
		static int i(1);
		auto a = G.NenhumRamoSingular();
		auto b = G.CaminhosFechados();
		auto c = G.TodosNodosConectados();
		cout << i++ << " ";
		cout << boolalpha << setw(9) << a.first << "  ";
		cout << boolalpha << setw(9) << b.first << "  ";
		cout << boolalpha << setw(9) << c << endl;
	};
	//com singulares
	{
		Grafo G(4);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,1);
		G.AdicionaAresta(3,2);
		imprime_testes(G);
	}
	{
		Grafo G(4);
		G.AdicionaAresta(3,3);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,2);
		G.AdicionaAresta(2,0);
		imprime_testes(G);
	}
	{
		Grafo G(4);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,1);
		G.AdicionaAresta(1,2);
		G.AdicionaAresta(3,2);
		imprime_testes(G);
	}
	{
		Grafo G(4);
		G.AdicionaAresta(3,3);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,2);
		G.AdicionaAresta(1,3);
		G.AdicionaAresta(2,0);
		imprime_testes(G);
	}
	//sem singulares
	{
		Grafo G(3);
		G.AdicionaAresta(1,2);
		imprime_testes(G);
	}
	{
		Grafo G(5);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,0);
		G.AdicionaAresta(2,3);
		G.AdicionaAresta(3,4);
		G.AdicionaAresta(4,2);
		imprime_testes(G);
	}
	{
		Grafo G(3);
		G.AdicionaAresta(1,2);
		G.AdicionaAresta(2,0);
		imprime_testes(G);
	}
	{
		Grafo G(5);
		G.AdicionaAresta(0,1);
		G.AdicionaAresta(1,3);
		G.AdicionaAresta(2,0);
		G.AdicionaAresta(2,3);
		G.AdicionaAresta(3,4);
		G.AdicionaAresta(4,2);
		imprime_testes(G);
	}
}
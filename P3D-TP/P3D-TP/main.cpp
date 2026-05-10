#include <iostream>
#include <array>

#include "Game.h"
#include "Camera.h"
#include "Light.h"
#include "Object.h"
#include "Shader.h"
#include "Renderer.h" // Inclui a classe 'Renderer', que define o renderizador a associar a cada objeto do jogo

// ------------------------------------------------------------
// É aqui que se incluem as suas classes de comportamento personalizados
// ------------------------------------------------------------
#include "Oscilator.h" // Inclui a classe 'Oscilator', que define o comportamento do objeto oscilador


// Usar um namespace mais curto para facilitar a escrita do código, e.g., 'gep3d::Game' em vez de 'game_engine_p3d::Game'
namespace gep3d = game_engine_p3d;


int main() {
	//using namespace gep3d;

	// NOTA: Ao definir para PT as definições regionais, alteramos a forma como o programa interpreta os números decimais (e.g., o separador decimal passa a ser a vírgula ',' em vez do ponto '.').
	//       Tal pode causar problemas na leitura de ficheiros de texto que contenham números decimais, como os shaders ou os modelos 3D (ficheiros .obj e outros).
	//       Por exemplo, se um shader ou modelo 3D contiver um número decimal como '0.5', o programa pode interpretar isso como '0,5' e não conseguir ler corretamente o valor, levando a erros de compilação do shader ou de carregamento do modelo.
	//       Assim, nas funções que realizam a leitura de ficheiros de texto que contenham números decimais, é importante garantir que o programa esteja a utilizar a locale correta.
	//       Devemos guardar a locale que estamos a utilizar no programa, e definir explicitamente a locale para "C" ou "en_US.UTF-8" (que usam o ponto como separador decimal) antes de ler os ficheiros de texto, e depois restaurar a locale original do programa.
	// Definições regionais (locale)
	try {
		// locale para português de Portugal
#ifdef __linux__
		std::locale::global(std::locale("pt_PT"));
#else
		std::locale::global(std::locale("pt-PT"));
#endif
	}
	catch (const std::exception& e) {
		std::cerr << "Erro ao definir locale: " << e.what() << std::endl;
	}

	// --------------------------------------------------
	// Cria uma instância do jogo
	// --------------------------------------------------
	gep3d::Game game(1200, 700);

	// --------------------------------------------------
	// Preparação da(s) câmara(s)
	// --------------------------------------------------
	// Instancia a câmara
	gep3d::Camera camera;
	// Define a cor de fundo da câmara
	camera.set_background_color(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	// Define a posição da câmara e o ponto de vista
	camera.LookAt(glm::vec3(16.0f, 10.0f, 40.0f), glm::vec3(16.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// Define a projeção perspetiva da câmara
	camera.Prespective(45.0f, static_cast<float>(game.width()) / game.height(), 0.1f, 100.0f);
	//camera.Orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f); // Define a projeção ortográfica
	// Define a viewport da câmara
	camera.Viewport(game.width(), game.height(), 0, 0);
	// Adiciona layers à máscara de culling da câmara
	std::array<std::string, 3> layers = { "Default", "Environment", "UI" };
	for (const auto& layer : layers) {
		camera.AddLayerToCullingMask(layer); // Adiciona a layer à máscara de culling da câmara
	}

	// Adiciona a câmara ao jogo (a primeira câmara adicionada é considerada a "Main Camera")
	game.AddCamera(&camera);

	// --------------------------------------------------
	// Preparação da(s) luzes(es)
	// --------------------------------------------------
	// Cria uma luz ambiente
	gep3d::Light* ambient_light = new gep3d::Light(glm::vec3(0.1f, 0.1f, 0.1f)); // Cor da luz ambiente
	// Cria uma luz direcional (ex: sol)
	gep3d::Light* directional_light = new gep3d::Light(
		glm::vec3(0.0f, 0.0f, -1.0f),	// Direção da luz
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente ambiente
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente difusa
		glm::vec3(1.0f, 1.0f, 1.0f)     // Componente especular
	);
	// Cria uma luz pontual (ex: lâmpada)
	gep3d::Light* point_light = new gep3d::Light(
		glm::vec3(0.0f, 0.0f, -1.0f),	// Posição da luz
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente ambiente
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente difusa
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente especular
		1.0f,                           // Constante de atenuação
		0.09f,                          // Linear de atenuação
		0.032f                          // Quadrática de atenuação
	);
	// Cria uma luz cónica (spotlight)
	gep3d::Light* spot_light = new gep3d::Light(
		glm::vec3(0.0f, 0.0f, -1.0f),  // Posição da luz
		glm::vec3(0.0f, 0.0f, -1.0f),   // Direção da luz
		glm::vec3(0.0f, 0.0f, 0.0f),    // Componente ambiente
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente difusa
		glm::vec3(1.0f, 1.0f, 1.0f),    // Componente especular
		1.0f,                           // Constante de atenuação
		0.09f,                          // Linear de atenuação
		0.032f,                         // Quadrática de atenuação
		10.0f,							// CutOff (ângulo interno, em graus)
		20.0f							// OuterCutOff (ângulo externo, em graus)
	);
	// Adiciona a luz ambiente ao jogo
	game.AddLight(ambient_light);
	// Adiciona a luz direcional ao jogo
	game.AddLight(directional_light);
	// Adiciona a luz pontual ao jogo
	game.AddLight(point_light);
	// Adiciona a luz cónica ao jogo
	game.AddLight(spot_light);

	// --------------------------------------------------
	// Preparação do(s) programa(s) shader
	// --------------------------------------------------
	// Para um programa shader, indica os tipos de shaders que serão usados e os respetivos caminhos para os ficheiros de código shader
	std::vector<ShaderSource> sources = {
		{GL_VERTEX_SHADER, "light.vert" /*"default_shader.vert"*/},
		{GL_FRAGMENT_SHADER, "light.frag" /*"default_shader.frag"*/}
	};
	// Cria o programa shader (lê e compila os shaders de um programa shader, a partir dos ficheiros especificados em 'sources')
	// O nome do shader é opcional, mas pode ser útil para identificação
	Shader* shader = new Shader(sources, "DefaultShader");

	// --------------------------------------------------
	// Preparação do(s) renderizador(es)
	// --------------------------------------------------
	// Cria o renderizador com o shader especificado e o caminho do modelo 3D (ficheiro OBJ)
	Renderer* renderer1 = new Renderer(shader, "Ball1.obj");
	Renderer* renderer2 = new Renderer(shader, "Ball2.obj");
	Renderer* renderer3 = new Renderer(shader, "Ball3.obj");
	Renderer* renderer4 = new Renderer(shader, "Ball4.obj");
	Renderer* renderer5 = new Renderer(shader, "Ball5.obj");
	Renderer* renderer6 = new Renderer(shader, "Ball6.obj");
	Renderer* renderer7 = new Renderer(shader, "Ball7.obj");
	Renderer* renderer8 = new Renderer(shader, "Ball8.obj");
	Renderer* renderer9 = new Renderer(shader, "Ball9.obj");
	Renderer* renderer10 = new Renderer(shader, "Ball10.obj");
	Renderer* renderer11 = new Renderer(shader, "Ball11.obj");
	Renderer* renderer12 = new Renderer(shader, "Ball12.obj");
	Renderer* renderer13 = new Renderer(shader, "Ball13.obj");
	Renderer* renderer14 = new Renderer(shader, "Ball14.obj");
	Renderer* renderer15 = new Renderer(shader, "Ball15.obj");


	// --------------------------------------------------
	// Preparação do(s) comportamento(s) do(s) objeto(s)
	// --------------------------------------------------
	// Cria uma instância do comportamento Oscilator
	Oscilator* oscilator = new Oscilator();

	// --------------------------------------------------
	// Preparação do(s) objeto(s) do jogo
	// --------------------------------------------------
	// Instancia um objeto do jogo
	// Cria um objeto com nome "Objecto (1)" e layer padrão ("" = "Default")
	// Atribui um comportamento 'oscilator' ao objeto, que será executado no ciclo de atualização do jogo
	// Atribui um renderizador ao objeto, que será usado para renderizar o objeto no jogo
	// Define a posição do objeto como (0, -4, 0), no sistema de coordenadas local, com orientação e escala padrão
	gep3d::Object* object1 = new gep3d::Object("Objecto (1)", "", nullptr, renderer1, 2.0f, 0.0f, 0.0f);
	gep3d::Object* object2 = new gep3d::Object("Objecto (2)", "", nullptr, renderer2, 4.0f, 0.0f, 0.0f);
	gep3d::Object* object3 = new gep3d::Object("Objecto (3)", "", nullptr, renderer3, 6.0f, 0.0f, 0.0f);
	gep3d::Object* object4 = new gep3d::Object("Objecto (4)", "", nullptr, renderer4, 8.0f, 0.0f, 0.0f);
	gep3d::Object* object5 = new gep3d::Object("Objecto (5)", "", nullptr, renderer5, 10.0f, 0.0f, 0.0f);
	gep3d::Object* object6 = new gep3d::Object("Objecto (6)", "", nullptr, renderer6, 12.0f, 0.0f, 0.0f);
	gep3d::Object* object7 = new gep3d::Object("Objecto (7)", "", nullptr, renderer7, 14.0f, 0.0f, 0.0f);
	gep3d::Object* object8 = new gep3d::Object("Objecto (8)", "", nullptr, renderer8, 16.0f, 0.0f, 0.0f);
	gep3d::Object* object9 = new gep3d::Object("Objecto (9)", "", nullptr, renderer9, 18.0f, 0.0f, 0.0f);
	gep3d::Object* object10 = new gep3d::Object("Objecto (10)", "", nullptr, renderer10, 20.0f, 0.0f, 0.0f);
	gep3d::Object* object11 = new gep3d::Object("Objecto (11)", "", nullptr, renderer11, 22.0f, 0.0f, 0.0f);
	gep3d::Object* object12 = new gep3d::Object("Objecto (12)", "", nullptr, renderer12, 24.0f, 0.0f, 0.0f);
	gep3d::Object* object13 = new gep3d::Object("Objecto (13)", "", nullptr, renderer13, 26.0f, 0.0f, 0.0f);
	gep3d::Object* object14 = new gep3d::Object("Objecto (14)", "", nullptr, renderer14, 28.0f, 0.0f, 0.0f);
	gep3d::Object* object15 = new gep3d::Object("Objecto (15)", "", nullptr, renderer15, 30.0f, 0.0f, 0.0f);

	// Cria um segundo objeto com nome "Objecto (2)" e layer padrão, sem comportamento, mas com o mesmo renderizador do primeiro objeto, e posiciona-o em (0, -2, 0)

	LOG("Object created with ID: " << object1->id() << " at position: (2, 0, 0).");
	LOG("Object created with ID: " << object2->id() << " at position: (4, 0, 0).");
	LOG("Object created with ID: " << object3->id() << " at position: (6, 0, 0).");
	LOG("Object created with ID: " << object4->id() << " at position: (8, 0, 0).");
	LOG("Object created with ID: " << object5->id() << " at position: (10, 0, 0).");
	LOG("Object created with ID: " << object6->id() << " at position: (12, 0, 0).");
	LOG("Object created with ID: " << object7->id() << " at position: (14, 0, 0).");
	LOG("Object created with ID: " << object8->id() << " at position: (16, 0, 0).");
	LOG("Object created with ID: " << object9->id() << " at position: (18, 0, 0).");
	LOG("Object created with ID: " << object10->id() << " at position: (20, 0, 0).");
	LOG("Object created with ID: " << object11->id() << " at position: (22, 0, 0).");
	LOG("Object created with ID: " << object12->id() << " at position: (24, 0, 0).");
	LOG("Object created with ID: " << object13->id() << " at position: (26, 0, 0).");
	LOG("Object created with ID: " << object14->id() << " at position: (28, 0, 0).");
	LOG("Object created with ID: " << object15->id() << " at position: (30, 0, 0).");


	// --------------------------------------------------
	// Adiciona o(s) objeto(s) ao jogo
	// --------------------------------------------------
	game.AddObject(object1);
	game.AddObject(object2);
	game.AddObject(object3);
	game.AddObject(object4);
	game.AddObject(object5);
	game.AddObject(object6);
	game.AddObject(object7);
	game.AddObject(object8);
	game.AddObject(object9);
	game.AddObject(object10);
	game.AddObject(object11);
	game.AddObject(object12);
	game.AddObject(object13);
	game.AddObject(object14);
	game.AddObject(object15);



	// --------------------------------------------------
	// Inicia o loop do jogo
	// --------------------------------------------------
	game.Run();

	// --------------------------------------------------
	// Liberta a memória alocada para os recursos do jogo
	// --------------------------------------------------
	delete shader;		// Liberta a memória alocada para o shader
	delete renderer1;	// Liberta a memória alocada para o renderizador
	delete renderer2;
	delete renderer3;
	delete renderer4;
	delete renderer5;
	delete renderer6;
	delete renderer7;
	delete renderer8;
	delete renderer9;
	delete renderer10;
	delete renderer11;
	delete renderer12;
	delete renderer13;
	delete renderer14;
	delete renderer15;
	delete oscilator;	// Liberta a memória alocada para o comportamento
	delete object1;		// Liberta a memória alocada para o objeto
	delete object2;		// Liberta a memória alocada para o objeto
	delete object3;
	delete object4;
	delete object5;
	delete object6;
	delete object7;
	delete object8;
	delete object9;
	delete object10;
	delete object11;
	delete object12;
	delete object13;
	delete object14;
	delete object15;

	LOG("Exit!");

	return 0;
}
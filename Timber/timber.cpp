
#include "pch.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>

using namespace sf;

const int NUM_BRANCHES = 6;
Sprite branches[NUM_BRANCHES];

//where is the player branch?
//Left or Right
enum class side {LEFT, RIGHT, NONE}; //Esta wea es magia aun para mi pero hay que ver como se usa porque parece ser re util
side branchPositions[NUM_BRANCHES];

//Funcione para mover las branches
void updateBranches(int seed);

int main()
{
    //Crea un objeto video
	VideoMode vm(960, 580);

	//Abrir la pantalla
	RenderWindow window(vm, "Timber!!!");

	//Genera una vista
	View view(FloatRect(0, 0, 1920, 1080));
	window.setView(view);

	//Objeto textura para tenerlos en gpu
	Texture textureBG;
	textureBG.loadFromFile("graphics/background.png");

	//Objeto esprite, necesita un objeto texture para funcionar
	Sprite spriteBG;
	spriteBG.setTexture(textureBG);

	//Posicion que tomará el sprite
	spriteBG.setPosition(0, 0);

	//New sprite, tree
	Texture textTree;
	textTree.loadFromFile("graphics/tree.png");
	Sprite spriteTree(textTree);
	spriteTree.setPosition(810, 0);

	//Preparar abeja
	Texture textBee;
	textBee.loadFromFile("graphics/bee.png");
	Sprite spriteBee(textBee);
	spriteBee.setPosition(0, 800);

	//Movimiento abeja
	bool beeActive = false; //En pantalla?
	float beeSpeed = 0.0f;

	//Tres nubes de una textura
	Texture textCloud;
	textCloud.loadFromFile("graphics/cloud.png");

	Sprite clouds[3];
	for (int i = 0; i < 3; i++) {
		clouds[i].setTexture(textCloud);
		clouds[i].setPosition(0, i*250);
	}

	bool activeClouds[3] = { false, false, false };
	float cloudsSpeed[3] = { 0.0f, 0.0f, 0.0f };

	//Para las letras
	//Cargar font
	Font font;
	font.loadFromFile("fonts/KOMIKAP_.ttf");

	int score = 0;

	//Generar variables de texto e inicialisarlas
	Text messageText;
	Text scoreText;

	messageText.setFont(font);
	scoreText.setFont(font);

	messageText.setString("Press enter to start!");
	scoreText.setString("Score = 0");

	messageText.setCharacterSize(75);
	scoreText.setCharacterSize(100);

	messageText.setFillColor(Color::White);
	scoreText.setFillColor(Color::White);

	//posicionar el texto previamente generado
	FloatRect textRect = messageText.getLocalBounds();

	messageText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

	scoreText.setPosition(20, 20);


	//variable para pausar el juego
	bool paused = true;

	//Variable para controlar el tiempo
	Clock clock;

	//Barra de tiempo 
	RectangleShape timebar;
	float timeBarStartWidth = 400;
	float timeBarHeight = 80;
	timebar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
	timebar.setFillColor(Color::Red);
	timebar.setPosition((1920 / 2) - timeBarStartWidth / 2, 980);

	//Esta wea hay que verla más TODO
	Time gameTimeTotal;
	float timeRemaining = 6.0f;
	float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

	//Prepare the branches
	Texture textureBranch;
	textureBranch.loadFromFile("graphics/branch.png");

	for (int i = 0; i < NUM_BRANCHES; i++) {
		branches[i].setTexture(textureBranch);
		branches[i].setPosition(-2000, -2000);
		//Origen en el centro del sprite, para poder girarlas de lo más mejor
		branches[i].setOrigin(220, 20);
	}

	//Codigo del jugador
	Texture texturePlayer;
	texturePlayer.loadFromFile("graphics/player.png");
	Sprite spritePlayer;
	spritePlayer.setTexture(texturePlayer);
	spritePlayer.setPosition(580, 720);

	side sidePlayer = side::LEFT;

	//Preparacion para la muerte
	Texture textureGravestone;
	Sprite spriteGravestone;
	textureGravestone.loadFromFile("graphics/rip.png");
	spriteGravestone.setTexture(textureGravestone);
	spriteGravestone.setPosition(600, 860);

	//Prepare axe
	Texture textureAxe;
	Sprite spriteAxe;
	textureAxe.loadFromFile("graphics/axe.png");
	spriteAxe.setTexture(textureAxe);
	spriteAxe.setPosition(700, 830);

	//Posiciones de el hacha
	const float AXE_POSITION_LEFT = 700;
	const float AXE_POSITION_RIGHT = 1075;

	//Pedazo de madera que vuela
	Texture textureLog;
	Sprite spriteLog;
	textureLog.loadFromFile("graphics/log.png");
	spriteLog.setTexture(textureLog);
	spriteLog.setPosition(810, 720);

	//Otras cosas utiles, que pertenecen al log
	bool logActive = false;
	float logSpeedX = 1000;
	float logSpeedY = -1500;

	//Input handler
	bool acceptInput = false;

	//SONIDOS!!!!!!
	SoundBuffer chopBuffer;
	SoundBuffer deathBuffer;
	SoundBuffer ootBuffer;

	chopBuffer.loadFromFile("sounds/chop.wav");
	deathBuffer.loadFromFile("sounds/death.wav");
	ootBuffer.loadFromFile("sounds/out_of_time.wav");

	Sound chop;
	Sound death;
	Sound outOfTime;

	chop.setBuffer(chopBuffer);
	death.setBuffer(deathBuffer);
	outOfTime.setBuffer(ootBuffer);

	while (window.isOpen()) {

		/*
		*****************************
		Handle the input
		*****************************
		*/

		//Here comes the magic!!!!!!!
		Event event;

		while (window.pollEvent(event)) {  //queue de eventos, por eso se hace el while, asi se sale cuando no existan más eventos
			if (event.type == Event::KeyReleased && !paused) {
				//Se puede presionar denuevo una tecla washito
				acceptInput = true;

				//Esconder la hacha
				spriteAxe.setPosition(2000, spriteAxe.getPosition().y);
			}
		}

		if (Keyboard::isKeyPressed(Keyboard::Escape)) {
			window.close();
		}

		//Start the game
		if (Keyboard::isKeyPressed(Keyboard::Return)) {
			paused = false;

			//Resetear variables necesarias
			score = 0;
			timeRemaining = 6.0f;

			//Hacer desaparecer las banches
			for (int i = 0; i < NUM_BRANCHES; i++) {
				branchPositions[i] = side::NONE;
			}

			//Esconder Gravestone
			spriteGravestone.setPosition(675, 2000);

			//Poner el player en la posicion inicial
			spritePlayer.setPosition(580, 720);

			acceptInput = true;
		}

		//Input handler, haciendo que no haga mil veces la misma cosa
		if (acceptInput) {
			//Flecha derecha presionada
			if (Keyboard::isKeyPressed(Keyboard::Right)) {
				//Mover al leñador a la derecha
				sidePlayer = side::RIGHT;
				score++;
				timeRemaining += (2 / score) + .15;

				spriteAxe.setPosition(AXE_POSITION_RIGHT, spriteAxe.getPosition().y);

				spritePlayer.setPosition(1200, 720);

				//Update la branch
				updateBranches(score);
				
				//Tirar log a la izq
				spriteLog.setPosition(810, 720);
				logSpeedX = -5000;
				logActive = true;

				acceptInput = false;

				//SONIDO
				chop.play();
			}

			//Fecha izquierda presionada
			if (Keyboard::isKeyPressed(Keyboard::Left)) {
				//Mover al leñador a la izquierda
				sidePlayer = side::LEFT;
				score++;

				timeRemaining += (2 / score) + .15;

				spriteAxe.setPosition(AXE_POSITION_LEFT, spriteAxe.getPosition().y);

				spritePlayer.setPosition(580, 720);

				//update Branches
				updateBranches(score);

				//Tirar log a la derecha
				spriteLog.setPosition(810, 720);
				logSpeedX = 5000;
				logActive = true;

				acceptInput = false;

				//SONIDO
				chop.play();
			}

		}

		/*
		*****************************
		Update shit
		*****************************
		*/
		if (!paused) {
			//Importante
			Time dt = clock.restart();

			//Mecanicas con tiempo
			timeRemaining -= dt.asSeconds();
			timebar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));

			if (timeRemaining <= 0.0f) {
				//Pausar el juego
				paused = true;
				messageText.setString("Se terminó el tiempo");
				FloatRect textRect = messageText.getLocalBounds();
				messageText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
				messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

				//SONIDO
				outOfTime.play();
			}

			//Weas graficas
			if (!beeActive) {
				srand((int)time(0));
				beeSpeed = (rand() % 200) + 200;


				srand((int)time(0) * 10);
				float height = (rand() % 500) + 500;
				spriteBee.setPosition(2000, height);
				beeActive = true;
			}
			else {
				spriteBee.setPosition(spriteBee.getPosition().x - (beeSpeed * dt.asSeconds()), spriteBee.getPosition().y);

				//Ver si llego a un lado izq de la pantalla
				if (spriteBee.getPosition().x < -100) {
					beeActive = false;
				}
			}

			for (int i = 0; i < 3; i++) {
				if (!activeClouds[i]) {
					//Velocidad random de la nube al aparecer
					srand((int)time(0) * 10 * (i + 1));
					cloudsSpeed[i] = (rand() % 200) + 1;

					//Altura al azar, posicion inicial y activacion
					srand((int)time(0) * 10 * (i + 1));
					float height = (rand() % 150);
					clouds[i].setPosition(-200, height);
					activeClouds[i] = true;
				}
				else {
					//mover nube
					clouds[i].setPosition(clouds[i].getPosition().x + (dt.asSeconds() * cloudsSpeed[i]), clouds[i].getPosition().y);
					
					//La nube salio de la pantalla
					if (clouds[i].getPosition().x > 1920) {
						activeClouds[i] = false;
					}
				}
			}			

			//update score
			std::stringstream ss;
			ss << "Score = " << score;
			scoreText.setString(ss.str());

			//update branches
			for (int i = 0; i < NUM_BRANCHES; i++) {
				float height = i * 150;
				if (branchPositions[i] == side::LEFT) {
					//Mover el sprite a la izquierda
					branches[i].setPosition(610, height);

					//flip el sprite
					branches[i].setRotation(180);
				}
				else if (branchPositions[i] == side::RIGHT) {
					//Mover el sprite a la derecha
					branches[i].setPosition(1330, height);

					//Poner la rotacion normal
					branches[i].setRotation(0);
				}
				else {
					//Esconde la branch
					branches[i].setPosition(3000, height);
				}
			}

			//handle flying log
			if (logActive) {
				spriteLog.setPosition(spriteLog.getPosition().x + (logSpeedX * dt.asSeconds()), 
					spriteLog.getPosition().y + (logSpeedY * dt.asSeconds()));

				//Revisar si el log salio por algun lado
				if (spriteLog.getPosition().x > 2000 || spriteLog.getPosition().x < -100) {
					//Setearlo para que sea un log nuevo
					logActive = false;
					spriteLog.setPosition(810, 720);
				}
			}

			//Handle death
			if (branchPositions[5] == sidePlayer) {
				//Muerte y destruccion
				paused = true;
				acceptInput = false;

				//Dibujar Gravestone
				spriteGravestone.setPosition(525, 760);
				
				//Esconder al jugador
				spritePlayer.setPosition(2000, 660);

				//Cambiar el mensaje de muerte
				messageText.setString("Muerto!");
				FloatRect textRect = messageText.getLocalBounds();

				messageText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

				messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

				//SONIDO
				death.play();
			}

		}//END if pause

		/*
		*****************************
		Draw the shit
		*****************************
		*/

		//Clear the shit
		window.clear();
		
		//Draw game scene here
		window.draw(spriteBG);

		for (int i = 0; i < 3; i++) {
			window.draw(clouds[i]);
		}

		for (int i = 0; i < NUM_BRANCHES; i++) {
			window.draw(branches[i]);
		}

		window.draw(spriteTree);

		window.draw(spritePlayer);

		window.draw(spriteAxe);

		window.draw(spriteLog);

		window.draw(spriteGravestone);
		
		window.draw(spriteBee);

		//Letritas, hud y homosexualidades varias
		window.draw(scoreText);
		if (paused) {
			window.draw(messageText);
		}
		window.draw(timebar);

		//Show everything we just drew
		window.display();

	}
	return 0;
}


void updateBranches(int seed) {
	//Mover todas las branches bajar un lugar
	for (int i = NUM_BRANCHES - 1; i > 0; i--) {
		branchPositions[i] = branchPositions[i - 1];
	}
	//Generar otra branch al inicio
	srand((int)time(0) + seed);
	int r = (rand() % 5);
	switch (r) {
	case 0:
		branchPositions[0] = side::LEFT;
		break;
	case 1:
		branchPositions[0] = side::RIGHT;
		break;
	default:
		branchPositions[0] = side::NONE;
		break;
	}

}
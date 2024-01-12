#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include <map>
#include <algorithm>


const int framerate_limit = 60;
const int sample_rate = 44100;
const int num_samples = 5*sample_rate;
sf::Int16 raw_samples[num_samples] {};
sf::SoundBuffer buffer;
sf::Sound sound;
bool samples_need_updating = true;

const int num_keys = 20;
const int num_white_keys = 12;
bool keys[num_keys] {false};

sf::Font font;
sf::Text labels[num_keys];

const int screen_w = 200;
const int screen_h = 70;

std::vector<sf::RectangleShape> keys_display(num_keys);

// given a key index, where do we find it in keys_display?
// all the white keys are at the front and the black keys at the back
std::vector<int> key_indices = {
   0, 12, 1, 13, 2, 3, 14, 4, 15, 5, 16, 6, 7, 17, 8, 18, 9, 10, 19, 11
};

sf::VertexArray key_seperators;

const std::map<sf::Keyboard::Key, int> keymap
{
   {sf::Keyboard::Q,        0 },    // C
   {sf::Keyboard::Num2,     1 },    // 
   {sf::Keyboard::W,        2 },    // D
   {sf::Keyboard::Num3,     3 },    // 
   {sf::Keyboard::E,        4 },    // E
   {sf::Keyboard::R,        5 },    // F
   {sf::Keyboard::Num5,     6 },    // 
   {sf::Keyboard::T,        7 },    // G
   {sf::Keyboard::Num6,     8 },    // 
   {sf::Keyboard::Y,        9 },    // A
   {sf::Keyboard::Num7,     10},    // 
   {sf::Keyboard::U,        11},    // B
   {sf::Keyboard::I,        12},    // C
   {sf::Keyboard::Num9,     13},    // 
   {sf::Keyboard::O,        14},    // D
   {sf::Keyboard::Num0,     15},    // 
   {sf::Keyboard::P,        16},    // E
   {sf::Keyboard::LBracket, 17},    // F
   {sf::Keyboard::Equal,    18},    // 
   {sf::Keyboard::RBracket, 19},    // G
};

bool isPianoKey(sf::Keyboard::Key key)
{
   switch (key)
   {
      case sf::Keyboard::Q:
      case sf::Keyboard::Num2:
      case sf::Keyboard::W:
      case sf::Keyboard::Num3:
      case sf::Keyboard::E:
      case sf::Keyboard::R:
      case sf::Keyboard::Num5:
      case sf::Keyboard::T:
      case sf::Keyboard::Num6:
      case sf::Keyboard::Y:
      case sf::Keyboard::Num7:
      case sf::Keyboard::U:
      case sf::Keyboard::I:
      case sf::Keyboard::Num9:
      case sf::Keyboard::O:
      case sf::Keyboard::Num0:
      case sf::Keyboard::P:
      case sf::Keyboard::LBracket:
      case sf::Keyboard::Equal:
      case sf::Keyboard::RBracket:
         return true;
      default: return false;
   }
}

void pollEvents(sf::RenderWindow& window) {
   sf::Event event;
   while (window.pollEvent(event))
   {
      switch (event.type)
      {
      case sf::Event::Closed:
         window.close();
         break;
      case sf::Event::KeyPressed:
      {
         if (!isPianoKey(event.key.code)) break;
         int idx = keymap.at(event.key.code);
         if (!keys[idx])
         {
            keys[idx] = true;
            samples_need_updating = true;
         }
         break;
      }
      case sf::Event::KeyReleased:
      {
         if (!isPianoKey(event.key.code)) break;
         int idx = keymap.at(event.key.code);
         if (keys[idx])
         {
            keys[idx] = false;
            samples_need_updating = true;
         }
         break;
      }
      default:
         break;
      }
   }
}

void updateSoundBuffer()
{
   double volume = 500.0;

   std::fill(&raw_samples[0], &raw_samples[0] + num_samples, 0);

   double scaling_factor = std::pow(2.0, 1.0/12.0); // key * 2^(1/12) gives the next key freq
   for (int key = 0; key < num_keys; key++)
   {
      if (not keys[key]) continue; // this key isn't pressed

      // figure out the frequency! key 0 is C, 3 steps above A at 
      // 440hz. Add 3 to every key to get how far above 440hz to go
      int steps_above_440 = key + 3;
      double freq = 440.0 * std::pow(scaling_factor, steps_above_440);

      for (int sample = 0; sample < num_samples; sample++)
      {
         raw_samples[sample] += volume * std::sin((sample * freq * 2 * M_PI) / sample_rate);
      }
   }

   samples_need_updating = false;
   buffer.loadFromSamples(raw_samples, num_samples, 1, sample_rate);
   sound.setBuffer(buffer);
   sound.setLoop(true);
   sound.play();
}

void updateKeyColors()
{
   for (int i = 0; i < num_keys; i++)
   {
      bool pressed = keys[i];
      int k = key_indices[i];
      bool white = k < num_white_keys;
      sf::RectangleShape& key = keys_display[k];

      if (not pressed)
      {
         key.setFillColor(white ? sf::Color::White : sf::Color::Black);
         continue;
      }

      if (white)
      {
         key.setFillColor(sf::Color(0x87eda2ff));
      }
      else
      {
         key.setFillColor(sf::Color(0x0e7d2bff));
      }
   }
}

void init_display()
{
   const char* input_str = "Q2W3ER5T6Y7UI9O0P[]=]";
   font.loadFromFile("Rubik-Regular.ttf");

   const float white_w = static_cast<float>(screen_w) / static_cast<float>(num_white_keys);
   const float white_h = screen_h;

   const float black_w = white_w * 0.6f;
   const float black_h = white_h / 2.f;

   float xoffset = 0;
   for (int i = 0; i < num_keys; i++)
   {
      int k = key_indices[i];
      sf::RectangleShape& key = keys_display[k];

      bool white = k < num_white_keys;

      float w = white ? white_w : black_w;
      float h = white ? white_h : black_h;

      key.setSize({w,h});
      key.setPosition({xoffset, 0.f});

      sf::Text& label = labels[i];
      label.setFont(font);
      label.setCharacterSize(12);
      label.setString(std::string(1, input_str[i]));
      label.setPosition({xoffset, 0.f});

      float label_w = label.getGlobalBounds().width;

      if (white)
      {
         xoffset += white_w;
         key.setFillColor(sf::Color::White);

         label.move({(white_w-label_w)/2.f, white_h - 18.f});
         label.setFillColor(sf::Color::Black);
      }
      else
      {
         key.move({-black_w/2.f, 0.f});
         key.setFillColor(sf::Color::Black);

         label.move({-label_w/2.f, black_h - 18.f});
         label.setFillColor(sf::Color::White);
      }
   }

   key_seperators.setPrimitiveType(sf::PrimitiveType::Lines);
   
   for (int i = 1; i < num_white_keys; i++)
   {
      key_seperators.append(sf::Vertex({i*white_w, 0.f}, sf::Color::Black));
      key_seperators.append(sf::Vertex({i*white_w, white_h}, sf::Color::Black));
   }
}

int main()
{
   sf::RenderWindow window(sf::VideoMode(screen_w, screen_h), "Piano", sf::Style::Default ^ sf::Style::Resize);
   window.setFramerateLimit(framerate_limit);
   

   init_display();

   updateSoundBuffer();
   buffer.loadFromSamples(raw_samples, num_samples, 1, sample_rate);
   sound.setBuffer(buffer);
   sound.setLoop(true);
   sound.play();

   while (window.isOpen())
   {
      pollEvents(window);
      if (samples_need_updating)
      {
         updateSoundBuffer();
         updateKeyColors();
      }

      window.clear();

      for (int i = 0; i < num_white_keys; i++)
      {
         window.draw(keys_display[i]);
      }
      window.draw(key_seperators);
      for (int i = num_white_keys; i < num_keys; i++)
      {
         window.draw(keys_display[i]);
      }
      for (int i = 0; i < num_keys; i++)
      {
         window.draw(labels[i]);
      }

      window.display();
   }

   return 0;
}
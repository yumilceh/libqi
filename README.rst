This is a recipe to compile aldebarab's libqi in Ubuntu 16.04 LTS for proyects in YASYT robotics. 

For more information see: http://github.com/aldebaran/libqi

Recipe
-----------

To compile libqi you need qibuild which give some cmake functions used
in libqi's CMakeLists.txt.

.. code-block:: sh

  pip2 install --user qibuild

  git clone git@github.com:aldebaran/libqi.git
  cd libqi

  mkdir BUILD && cd BUILD
  cmake .. -DQI_WITH_TESTS=OFF
  make
  make install DESTDIR=./output

.. code-block:: console

  ./myservice --qi-standalone # for a standalone server
  ./myservice --qi-url tcp://somemachine:9559 # to connect to another galaxy of sessions

Links
-----

git repository:
http://github.com/aldebaran/libqi

Documentation:
http://doc.aldebaran.com/libqi/

IRC Channel:
#qi on freenode.

Maintainers:

- Joël Lamotte <jlamotte@aldebaran.com>
- Jérémy Monnon <jmonnon@aldebaran.com>
- Matthieu Paindavoine <matthieu.paindavoine@softbankrobotics.com>
- Vincent Palancher <vincent.palancher@external.softbankrobotics.com>

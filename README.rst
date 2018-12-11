This is a recipe to compile aldebarab's libqi in Ubuntu 16.04 LTS for proyects in YASYT robotics. 

For more information see: http://github.com/aldebaran/libqi

Recipe
-----------

First you should uninstall install an old version of boost (which is compatible with libqi).
.. code-block:: sh
  sudo apt-get --purge remove libboost-dev libboost-doc
  wget -O boost_1_56_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.56.0
  tar xzvf boost_1_56_0.tar.gz
  rm boost*.tar.gz
  sudo apt-get update
  sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev build-essential libbz2-dev libboost-all-dev
  ./bootstrap.sh --prefix=/usr/
  ./b2
  sudo ./b2 install


Then continue with the instructions provided by softbank but using this repository.
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

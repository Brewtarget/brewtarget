Using Vagrant
=============

Vagrant allows for easily starting a VM in a known state.  This is perfect for
development because it reduces the amount of manual configuration needed to get
started, and helps ensure you don't introduce accidental dependencies.

Requirements
------------
  * Install [vagrant](https://www.vagrantup.com/docs/installation/)
  * Install [ansible](https://docs.ansible.com/ansible/intro_installation.html)


Usage
-----
In the project directory, run `vagrant up`.  Vagrant will download the required
VM image and configure it using Ansible.  The resulting VM will have Postgres
9.5 installed with a brewtarget database & user.  A default brewtarget.conf
which uses this Postgres database is installed for the vagrant user.

Once vagrant has build the machine you can access it via `vagrant ssh`.
Because brewtarget is a GUI application, you will probably want to have
XForwarding back to your workstation.  A script to do this is provided as
`vagrant/login`.

The project directory will be automatically mounted at `/vagrant`.  Builds can
be done in that directory using the normal toolchain.

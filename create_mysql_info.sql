-- create database named filter_config
create database if not exists filter_config;

use filter_config;




-- create table named AppConfig
create table if not exists T_AppConfig (
	ID			int	auto_increment,
	App_ID		int,
	Bandwidth	int,
	BeginTime	int,
	EndTime		int,
	primary		key(ID)
);

insert into T_AppConfig values (NULL, 7, 0, -1, 0);
insert into T_AppConfig values (NULL, 8, 0, -1, 0);


-- create table R_LevelAppConfig
create table if not exists R_LevelAppConfig (
	Level			int,
	AppConfig_ID	int
);

insert into R_LevelAppConfig values (1, 1);
insert into R_LevelAppConfig values (1, 2);

-- create table T_UserLevel
create table if not exists T_UserLevel (
	UserIP char(16)  primary key,
	Level int
);

insert into T_UserLevel values ("192.168.56.4", 1);
insert into T_UserLevel values ("192.168.56.5", 2);

create table T_SessionInfo (
	inIP		char(16),
	exIP		char(16),
	inPort		int,
	exPort		int,
	protocol	char(4),
	begin		bigint,
	end			bigint,
	appID		int,
	packets		bigint,
	bytes		bigint
);

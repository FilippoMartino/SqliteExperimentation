PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE LaMiaPrimaTabella(nome varchar(30), cognome varchar(30));
INSERT INTO LaMiaPrimaTabella VALUES('Filippo','Martino');
INSERT INTO LaMiaPrimaTabella VALUES('Luca','Tortore');
INSERT INTO LaMiaPrimaTabella VALUES('Anna','Martino');
COMMIT;

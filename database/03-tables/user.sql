CREATE TABLE "user"
(
  id smallint NOT NULL,
  login character varying(50) NOT NULL,
  email character varying(100) NOT NULL,
  role_id smallint NOT NULL,
  site_id smallint,
  password text NOT NULL,
  CONSTRAINT user_pkey PRIMARY KEY (id),
  CONSTRAINT user_fk FOREIGN KEY (role_id)
      REFERENCES role (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT user_fk1 FOREIGN KEY (site_id)
      REFERENCES site (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT user_login_key UNIQUE (login)
);

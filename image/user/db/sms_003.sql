-- Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
-- For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

BEGIN TRANSACTION;
INSERT OR REPLACE INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (1,1,1,1547465101,1,1,'najstarsze odb fsjdklafjskldjf',4);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (2,1,1,1547468701,2,2,'wys',8);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (4,1,1,1547472320,4,1,'najnowsze odb i całkiem 1 1 1 1 długie. długie długie długie, wcale nie krótkie.
od nowej lini (\n); i_teraz_zbyt_długa_linia_żeby_się_zmieścić',4);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (5,1,1,1547482320,3,2,'(dziwne znaczki: „”ẃ½€≠§³¢)',8);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (6,1,1,1547492320,3,2,'nieudane wysyłanie :(',2);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (7,2,2,1547492320,3,0,'Ciężko powiedzieć o czym  ta wiadomość jest, ale jest dość długa.',2);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (8,2,2,1547492320,4,0,'heh?',4);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (9,3,3,1547492320,1,0,'ping',2);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (10,3,3,1547492320,2,0,'pong',4);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (11,4,4,1547492320,2,0,'ping',4);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (12,5,5,1547492320,2,0,'ping',2);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (13,6,6,1547492320,2,0,'ping',8);
INSERT OR REPLACE  INTO "sms" ("_id","thread_id","contact_id","date","date_send","error_code","body","type") VALUES (14,1,1,1547472301,3,2,'prawie najnowsze odb.',4);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (1,'Thanks for reaching out. I can''t talk right now, I''ll call you later',4);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (2,'I''ll call you later',3);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (3,'I''ll be there in 15 minutes',2);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (4,'Some test tmplate number, which is too long to be displayed.',1);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (5,'No.',0);
INSERT OR REPLACE  INTO "templates" ("_id","text","lastUsageTimestamp") VALUES (6,'Zadzwonię później, walczę z ostrym cieniem mgły ;)',NULL);
INSERT OR REPLACE  INTO "threads_count" ("_id","count") VALUES (1,2);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (1,1574335694,6,1,1,1,'Wiadomość testowa.',1);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (2,2,2,0,2,2,'How are You?
',2);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (3,3,2,1,3,3,'Lorem ipsum dolor sit amet, consectetur adipiscing',4);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (4,4,1,0,4,4,'Lorem ipsum dolor sit amet, consectetur adipiscing',8);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (5,5,1,1,5,5,'Lorem ipsum dolor sit amet, consectetur adipiscing',16);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (6,5,1,0,6,6,'cos tam',8);
INSERT OR REPLACE  INTO "threads" ("_id","date","msg_count","read","contact_id","number_id","snippet","last_dir") VALUES (7,1,1,1,7,7,'cdcd',4);
COMMIT;

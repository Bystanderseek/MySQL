# MySQL

## 一. 功能

1. 建表
2. 删表
3. 描述表的属性
4. 展示数据库中的表
5. 表中数据与外存的交互
6. 向表中插入元组
7. 从表中删除元组
8. 单关系的投影，选择操作
9. 两个关系的投影，选择，连接操作
10. 表的格式化输出

## 二.语句

==括号匹配，单引号匹配，命名非法，缺少分号，指令不存在均已处理。==

### 1. Show tables;



​		*show tables;*



+ **合法输入**
  + show tables;
+ **非法输入**
  + show table;					//报错
  + show table abc;  		  //报错

### 2. Desc；



 		*desc <表名>;*



+ **合法输入**
  + desc student;
+ **非法输入**
  + desc stu;		//表名不存在，报错

### 3. Quit;



​		*quit;*



+ **合法输入**
  + quit;
+ **非法输入**
  + quit a;

### 4. Create；



​	*create table <表名> (<字段名> <字段类型>， <字段名><字段类型>...     primarykey(<属性名>));*

​	*create table <表名> (<字段名> <字段类型>， <字段名><字段类型>...);*



​	==主键关键字必须放在末尾，字段类型只支持 int, char, char的长度可以自定义。字段名必须合法（不能起名为关键字的名字，同时符合命名规则）。字段名合法规则与C++中变量名规则相同。（首位不能是数字，必须是下划线或者数字，除了首位其他位置只能是字母，数字或者下划线），如违反一条规则就会报错。在建表成功后，会在Database的文件夹中创建一个以表名命名的文件夹，其中包含该表的字段文件。当向表中插入值时，还会创建该表的数据文件。建表成功后会像表的数据字典中加入该表名，以后运行数据库的时候会根据字典中的名字读取表。==



+ **合法输入：**

  + create table student(num int, name char(10), age int, primarykey(num));
  + create table book(name char(10), price int);

+ **非法输入:**

  + 主键位置错误			//报错，主键位置不正确

    create table test_3 (name char(10), primarykey (name), age int);

  + 主键缺少括号			//报错，主键位置不正确

    create table test_3 (name char(10), age int, primarykey name);

  + 主键缺少名字			//报错，无效的类型（把primarykey解析成了字段名，它缺少字段属性）

    create table test_3 (name char(10), age int，primarykey);

  + 主键名称非法			//报错，主键名称不存在

    create table test_3(age int, name char(6),  primarykey (namee));

  + 多余的逗号				//报错，无效的类型

    create table test_3 (name char(10), age int，primarykey(name),);

  + 字段名非法				//报错，char和逗号不能作为变量名

    create table test_3(char char(10), age int);

    create table test_3(, char(10), age int);

  + 字段类型非法			//报错，无效的类型

    create table test_3(name varchar(10), age int);

  + 符号非法（出现中文符号）	//报错，字段类型非法

    create table test_3 (name char(10), age int，primarykey name);

  + 创建的表名非法			//报错

    create table 1(num int, name char(10), age int, primarykey(num));

  + 创建的表已经存在		//提示表已存在

​			  create table student(num int, name char(10), age int, primarykey(num));

### 5. Drop；



​	*drop table <表名>*



​	==成功删除某个表后，同时会在外存中删除该表的文件夹和文件夹下的所有子文件。==

+ **合法输入**
  + drop table book;
+ **非法输入**
  + drop table bokkk;		//提示表名不存在

### 6. Insert；



​	*insert into <表名> values(<值1>, <值2>...)；*



​	==只支持这一种形式的插入语句，插入的元组中值的顺序与建表时一致。如果提供的元组属性过多或过少都会报错，属性类型不对也会报错。如果要插入的表中有主键，则还会在插入之前先判断要插入的元组的主键是否已经存在，若存在，则插入失败并报错。（在插入时允许批量插入数据）除了插入可以批量插入之外，其他操作一律不允许批量操作。==

+ **合法输入**

  + insert into book values('helloworld', 10);

+ **非法输入**

  + 语句错误	//报错

    insert into book value('helloworld', 10);

  + 值类型不匹配	//报错

    insert into book values(helloworld, 10);

    insert into book values(helloworld, hello);

  + 属性过多或者过少	//报错，属性不匹配

    insert into book values('helloworld', 10, 20);

    insert into book values('helloworld');

  + 属性类型超过限制	//报错，类型不匹配

    insert into book values('helloworldwwwwwwwww', 10);

  + 无意义输入	//报错

    insert into book (20 = '20');

  + 单引号匹配错误	//报错，括号不匹配

    insert into book values('helloworl, 10);

  + 要插入的表名不存在	//报错，表明不存在

    insert into bookkkk values('helloworld');

  + 要插入的表中有主键，且主键已经存在	//报错，主键重复

    insert into student values(1, 'Jack', 20， 'chinese');

### 7. Delete；



​	*delete from <表名>;*

​	*delete from <表名> where <条件表达式>*



​	==删除时如果不给条件，就是删除所有。给的条件最多只有一个，不支持多条件，否则报错。删除之后，会同时更新外存表文件中的内容。==



+ **合法输入**

  + delete from book where name = 'helloworld';
  + delete from book where price < 9;
  + delete from book；

+ **非法输入**

  + 语句残缺	//报错

    delete from book where;

  + 表不存在	//报错，表不存在

    delete from bookkkk;

  + 条件残缺	//报错

    delete from book where name;

  + 字段不匹配	//报错，未识别的字段

    delete from book where name = 20;

  + 不存在的字段	//报错，未识别的字段

     delete from book where nammm = '20';

  + 无意义的条件	//报错

    delete from book where name < '20';

    delete from book where 20 < '20';

    delete from book where 20 < name;

  + 过多的语句

    delete from book where price < 9 and name = 'java';

### 8. Select；

#### （1）单表



​	*select \* from <表名>;*

​	*select  <属性名列表>  from <表名>;*

​	*select \* from <表名> where <条件表达式>;*

​	*select <属性名列表> from <表名> where <条件表达式>;*

​	*select \* from <表名> where <条件表达式1> and <条件表达式2>;*

​	*select \<属性名列表> from <表名> where <条件表达式1> and <条件表达式2>;*



​	==where 条件表达式的左侧条件必须是属性，比如 age = 20，而不能是 20 = age==



* **合法输入**

  select * from student;

  select cname, cnum, sname from course;

  select * from student where age < 20;

  select name from student where age > 20;

  select * from student where age < 20 and name = 'ash';

  select name, num from student where age < 20 and name = 'ash';

+ **非法输入**

  + 表名不存在	//报错，表不存在

    select * from s;

  + 属性不存在	//报错，未知字段

    select n from student;

  + 语句残缺		//报错

    select num from student where;

    select num from student where and;

    select num from student where name = 'buck' and;

    select num from student where name = 'buck' and num;

    select * from student where name = 'ash' age < 18;

  + 无意义的输入	//报错

    select ** from student;

    select from where;

    select * from student where 20 = 30;

    select * from student where 20 = '30';

  + 属性位置错误	//报错

    select * from student where 'ash' = name;

  + 类型不匹配		//报错

    select * from student where name = name;

    select * from student where age = name;

    select * from student where age = 'name';

    select * from student where name < 20;

  + 自身匹配

    select * from student where name = name;

    

#### （2）双表



​	*select \* from <表1>， <表2>;*

​	*select \* from <表1>，<表2>  where <条件表达式>;*

​	*select <属性名列表> from <表1>，<表2> where <条件表达式>;*

​	*select \* from <表1>，<表2> where <条件表达式1> and <条件表达式2>;*

​	*select \<属性名列表> from <表1>，<表2> where <条件表达式1> and <条件表达式2>;*



​	==如果涉及了两个表的带条件的操作，那么条件中，至少有一个是连接条件，并且等号左侧的是第一个表的，右侧的条件是第二个表的，否则报错。选择两个表进行映射操作时，如果是无条件的输出，那么属性名列表中的元素必须两表都有，否则报错。不允许自然连接，如果执行自然连接的语句，会分别输出两个表中的内容。==

* **合法输入**

​		select * from student, book;

​		select * from student, course where name = sname;

​		select name, num from student, course where name = sname;

​		select * from student, course where name = sname and age < 20;

​		select * from student, course where name = sname and favc = cname;

+ **非法输入**

  + 表不存在

    select * from student, bookkk;

  + 缺少连接条件	//报错，缺少连接条件或属性不匹配（其中一个表中没有此条件）

    select * from student, course where name = 'ash';

    select * from student, course where name = 'ash' and cname = 'chinese';

  + 无意义输入

    select * from student, course where 20 = 30 and name = '';

    select * from student, course where name = name and name = '';

  + 表名不存在

    select * from student, cours where sname = name and name = 'ash';


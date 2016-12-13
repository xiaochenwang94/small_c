#smallc
---
##测试说明
| 文件名称       | 内容           | 测试结果  |
| :------------- |:-------------:| -----:|
| test0.txt     | 100以内的素数   | pass  |
| test1.txt      | gcd      |   pass |
| zebra stripes | are neat      |    $1 |

---
##Error说明
| error num| 内容           				|   
| :--------|:-------------					|
| 1        | program begin error     		|
| 2        | statement begin error     	|   
| 3  	   | if_stmt begin error      		|   
| 4 	   | if lost then					|	
| 5 	   | in then statement begin error |
| 6 	   | if end else error				|
| 7 	   | in else statement begin error |
| 8		   | untilsym error					|
| 9 	   | assign error					|
| 10 	   | only variable can be assigned|
| 11 	   | sym follows read must be ident|
| 12	   | sym follows write must be ident|
| 13       | sym follow end must be semicolon|
| 14	   | sym follows write_stmt must be semicolon |
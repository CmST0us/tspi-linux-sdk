[global]
strict init = true
reload conf period = 5M

buffer min = 1024
buffer max = 1MB

rotate lock file = self
default format = "%d(%F %T.)%-3ms %p %t %-6V [%f:%L:%U] - %m%n"

file perms = 666
fsync period = 1K

[levels]
WAKE = 35
NLP =  30
ASR =  25

[formats]
simple_1 = "%m"
simple_2 = "%d([%F] [%T.)%-3ms] [%p] [%-6V] %m%n"
simple_3 = "%d([%F] [%T.)%-3ms] [%-6V] %m%n"
simple_4 = "%m%n"
normal = "%d([%F] [%T.)%-6us] [%p] [%-6V] [%f:%U:%L]%m%n"

[rules]
my_cat.=DEBUG		>stdout; simple_3
#my_cat.=WAKE		"/tmp/wake.log",5MB*2;simple_4
#my_cat.=NLP		"/tmp/nlp.log",10MB*2;simple_1
#my_cat.=ASR		"/tmp/asr.log",5MB*2;simple_4
my_cat.INFO		"/tmp/sai.log",10MB*1;normal

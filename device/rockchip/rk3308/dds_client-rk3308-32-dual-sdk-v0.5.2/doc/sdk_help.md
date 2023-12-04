## linux-sdk 使用说明

### 接口说明

**sdk语音相关的接口**

```
sdk 初始化函数，主要包括sdk参数初始化和授权两个过程，这个函数是阻塞的，
授权的超时时间为 15 秒。 

struct dds_client *dds_client_init (const char *config_json);

参数说明:

@ config_json: 配置选项，json 格式的字符串；

返回值:

错误情况下返回NULL, 否则返回 struct dds_client 实例指针；
```
```
运行 sdk 函数，调用此函数之后就可以语音交互了。

int dds_client_start(struct dds_client *ds, ddsLintener cb, 
void *user);

参数说明:

@ ds: 由 dds_client_init 返回的 struct dds_client 实例指针；
@ cb: 监听 sdk 事件的回调函数；
@ user: 用户参数；

返回值:

出错返回 -1

```

```
释放 sdk 实例的函数:

void dds_client_release(struct dds_client *ds);

参数说明:

@ ds: 由 dds_client_init 返回的 struct dds_client 实例指针；

```

**下面的接口必须在 `dds_client_init ` 和 	`dds_client_start` 正确返回之后才能正确执行。**

```
向 sdk 内部发送消息: 

int dds_client_publish(struct dds_client *ds, int ev, 
const char *data);

参数说明: 
@ ds: 由 dds_client_init 返回的 struct dds_client 实例指针；
@ ev: 发送的消息事件；
@ data: 此消息附带的数据, json 格式；


返回值: 

只有当 sdk 完成初始化并且授权成功之后才能正确返回，否则返回 -1 

```

```
返回 nativeAPI 查询结果的接口

int dds_client_resp_nativeapi(struct dds_client *ds, 
const char *native_api, const char *native_api_data_json);

参数说明:

ds: sdk 实例指针；

native_api: 这个 nativeAPI topic 名；

native_api_data_json: 查询数据结果，json string，格式如下，
"duiWidget" 字段必须包含， 且目前取值为 "text"。 用户自定义的数据必须放在
extra 字段中。

{
	"duiWidget": "text",
	"extra": {
		"xx": "11"
	}
}

返回值: 如果 sdk 没有初始化完成或者授权成功则返回-1

```

```
内部合成的接口: 

int dds_client_speak(struct dds_client *ds, const char *text);

参数说明: 

ds: sdk 实例指针

text: 需要合成的文本

返回值: 如果 sdk 没有初始化完成或者授权成功则返回-1
```

```
外部 feed 音频接口: 

int dds_client_feed_audio(struct dds_client *ds, char *data, int len);

参数说明: 

ds: sdk 实例指针

data: 录音机数据

len: 数据长度

返回值： 出错返回 -1 ，此接口只有在 recorder 配置为外部方式才会生效。 


```

```
停止当前对话，包括停止合成，取消识别等。

int dds_client_stop_dialog(struct dds_client *ds);

参数说明: 

ds: sdk 实例指针

text: 需要合成的文本

返回值: 如果 sdk 没有初始化完成或者授权成功则返回-1

```

```
关闭唤醒，如果在语音对话过程中调用此接口，会在这条对话自然结束之后才会禁止唤醒。

int dds_client_disable_wakeup(struct dds_client *ds);

参数说明: 

ds: sdk 实例指针

返回值: 如果 sdk 没有初始化完成或者授权成功则返回-1

```

```
打开唤醒

int dds_client_enable_wakeup(struct dds_client *ds);

参数说明: 

ds: sdk 实例指针

返回值: 如果 sdk 没有初始化完成或者授权成功则返回-1

```

```
设置用户唤醒词

int dds_client_update_customword(struct dds_client *ds, 
const char *word);

参数说明: 
ds: sdk 实例指针
word: 唤醒词配置，格式是 json string，说明如下: 

	{
            "greetingFile":"path:../res/tts/help.mp3", 可选
            "greeting": "我在，有什么可以帮你", 可选
            "pinyin": "ni hao xiao chi", 必选
            "name": "你好小驰", 必选
            "threshold": 0.127 必选
        }

此函数成功返回后，唤醒词的相关配置会更新到 config.json 文件。 
对于客户端异常断电可能导致 config.json 文件破坏的话， 需要开发者自己来避免，
比如采用备份文件的机制。 

```

```
获取当前的唤醒词

char* dds_client_get_wakeupwords(struct dds_client *ds);

参数说明: 
ds: sdk 实例指针

此函数返回字符串指针， 开发者需要主动释放内存。 返回字符串为json格式，如下: 

{
	"majorword": [{
		"greetingFile": "path:../res/tts/help.mp3",
		"greeting": "我在，有什么可以帮你",
		"pinyin": "ni hao xiao le",
		"name": "你好小乐",
		"threshold": 0.144000
	}],
	"minorword": [{
		"greetingFile": "path:../res/tts/help.mp3",
		"greeting": "我在，有什么可以帮你",
		"pinyin": "ni hao xiao chi",
		"name": "你好小驰",
		"threshold": 0.127000
	}],
	"cmdword": [{
		"pinyin": "jiang di yin liang",
		"threshold": 0.100000,
		"action": "decrease.volume",
		"name": "降低音量"
	}],
	"customword": [{
		"pinyin": "ni hao tiam mao",
		"name": "你好天猫",
		"threshold": 0.200000
	}]
}

majorword 为主唤醒词，minorword 为副唤醒词， cmdword 为命令词，
customword 为用户定义唤醒词。 其实就是 config.json 文件里面的配置。

```

```

// 获取当前的 tts 发音人，出错返回 NULL
char *dds_client_get_speaker(struct dds_client *ds);

// 获取当前的 tts 播报速度，为 float 型， 在 0 ~ 1 之间，越大表示速度越慢。 
float dds_clent_get_speed(struct dds_client *ds);

// 获取当前的 tts 的播报音量大小， 为 int 型， 在 0 ~ 100 之间。 
int dds_client_get_volume(struct dds_client *ds);

// 设置当前的 tts 的播报音色人，出错返回 -1
int dds_client_set_speaker(struct dds_client *ds, char *speaker);

// 设置当前的 tts 的播报速度大小，出错返回 -1 
int dds_client_set_speed(struct dds_client *ds, float speed);

// 设置当前的 tts 的播报音量大小，出错返回 -1 
int dds_client_set_volume(struct dds_client *ds, int vol);

```

```
声纹的相关接口

// 获取当前的声纹详细信息

char *dds_client_vprint_get_detail(struct dds_client *ds);

返回的是一个　json 格式字符串，格式为: 

{
	"vNum": 2,
	"detail": [
	{
		"name":"test1"
	},
	{
		"name":"test2"
	}
	]
}


// 开始进入声纹注册的接口

int dds_client_vprint_regist(struct dds_client *ds, char *name);

name 参数表示声纹注册人的姓名标识。

出错返回-1，通常是格式错误。而声纹注册过程中的详细错误将会通过回调抛出。


// 删除注册人信息

int dds_client_vprint_unregist(struct dds_client *ds, char *name);

出错返回-1，通常是格式错误。而声纹删除过程中的详细错误将会通过回调抛出。

```

```
能量接口

int dds_client_energy_estimate(struct dds_client* ds, int second);

second 参数表示接下来所要计算的音频时间长度，单位为秒。计算结果通过回调给出。

出错返回　-1 

```

```
客户端信息上传的相关接口

int dds_client_upload_city(struct dds_client *dc, char *city);

city 为当前设备所在的城市名，比如苏州，上海等

出错返回　-1 
```

**sdk回调消息接口**

<table>
<tr>
	<th>回调函数</th>
	<th>消息</th>
	<th>含义</th>
	<th>参数</th>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> local_wakeup.result  </td>
	<td> 唤醒事件 </td>
	<td> json string，形如 
	{"type":"major","greeting":"好的",
	"word":"你好小驰"} </td>
</tr>


<tr>
	<td> ddsLintener </td>
	<td> doa.result  </td>
	<td> doa事件 </td>
	<td> json string, 形如 {"dao": 100} </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.vad.begin  </td>
	<td> vad开始的事件 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.vad.end  </td>
	<td> vad结束的事件 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.tts.begin  </td>
	<td> 合成音开始的事件 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.tts.end  </td>
	<td> 合成音结束的事件，播放结束 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.asr.begin  </td>
	<td> sdk内部开始做识别 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> asr.speech.text  </td>
	<td> 实时的语音识别结果反馈 </td>
	<td> json string </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> asr.speech.result  </td>
	<td> 最终的语音识别结果反馈 </td>
	<td> json string </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> dm.output  </td>
	<td> 对话的输出结果 </td>
	<td> json string </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.dm.end  </td>
	<td> 表示结束对话 </td>
	<td> 无 </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> device.mode.return  </td>
	<td> 表示设置设备状态的回复消息 </td>
	<td> json string "{"result":"success"}"</td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> sys.client.error  </td>
	<td> 表示客户端出现异常情况 </td>
	<td> json string "{"error":"ttsError"}" 目前 error 字段的取值一共有: ttsError, ddsNetworkError, vadSlienceTimeout</td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> command://xx  </td>
	<td> 在dui平台上配置的 command 指令 </td>
	<td> json string </td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> native://xx  </td>
	<td> 在dui平台上配置的 native 指令 </td>
	<td> json string </td>
</tr>


<tr>
	<td> ddsLintener </td>
	<td> vprint.regist.result  </td>
	<td> 声纹注册的消息回调 </td>
	<td> json string  形如, {"operation":"start"} 表示声纹注册接口调用成成功，开始注册声纹。 {"operation":"nameRepeat"} 表示注册人姓名重复， {"operation":"vNumLimit"} 表示超出声纹注册上限， {"operation":"unavailable"} 表示所注册音频信噪比不够，  {"operation":"continue"} 表示可以继续注册声纹，{"operation":"success"}表示声纹注册成功</td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> vprint.unregist.result  </td>
	<td> 声纹删除的消息回调 </td>
	<td> json string  形如 {"operation":"success"} 表示删除成功, {"operation":"noSpeaker"}  表示没有所有删除的注册人信息</td>
</tr>

<tr>
	<td> ddsLintener </td>
	<td> vprint.test.result  </td>
	<td> 声纹计算的消息回调 </td>
	<td> json string 形如: {"register":"nothing"} 表示当前还没有声纹模型， {"score":23.286682,"word":"qi ke kong tiao","register":"test0","time":179.679932,"speech":0.880000,"RTF":0.204188} 表示收到了正常的计算结果，其中的  register 字段表示所计算的声纹标识。 如果  register 为 others表示没有匹配到具体的声纹</td>
</tr>


<tr>
	<td> ddsLintener </td>
	<td> energy.estimate.result </td>
	<td> 能量计算的消息回调 </td>
	<td> json string 形如, {"value":40} </td>
</tr>


</table>

### 配置选项

```

{
	"sdk": {
		"configPath":"./config.json"
	},
	"auth": {
		"productId": "278569448",
		"deviceProfile": ""
	},
	"front": {
		"aecBinPath": "",
		"wakeupBinPath": "",
		"beamformingBinPath": "",
		"rollBack": 0
	},
	"vad": {
		"resBinPath": "",
		"pauseTime": 500,
		"slienceTimeout": 5
	},
	"cloud": {
		"productId": "278569448",
		"aliasKey": "prod"
	},
	"recorder": {
	     "mode": "internal"
        "samplerate":16000,
        "bits":16,
        "channels":1,
		"device": "default"
	},
	"player": {
		"device": "default"
	},
	"tts": {
		"type": "cloud",
		"zhilingf": {
			"resBinPath":"",
			"dictPath":""
		},
		"voice": "zhilingf",
		"volume": 50,
		"speed": 0.85
	},
	"oneShot": {
		"enable": false
	},
	"abnormal": {
		"netErrorHint":"path:../res/tts/net.mp3",
		"ttsErrorHint":"path:../res/tts/tts_error.mp3"
	},
	"debug": {
		"recAudioDumpFile":"",
		"bfAudioDumpFile":""
	}
}
```

<table>
<tr>
	<th>参数</th>
	<th>类型</th>
	<th>含义</th>
	<th>是否必须</th>
</tr>

<tr>
	<td> sdk </td>
	<td> json 对象 </td>
	<td> 客户端的一些配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td> sdk.configPath </td>
	<td> string </td>
	<td> 配置文件路径 </td>
	<td>必选</td>
</tr>

<tr>
	<td> auth </td>
	<td> json 对象 </td>
	<td> 授权信息 </td>
	<td>必选</td>
</tr>

<tr>
	<td> auth.productId </td>
	<td>string</td>
	<td> dui 上创建产品ID </td>
	<td>必选</td>
</tr>

<tr>
	<td> auth.deviceProfile </td>
	<td>string</td>
	<td>授权信息</td>
	<td>必选</td>
</tr>

<tr>
	<td> front </td>
	<td>json 对象 </td>
	<td> 前端信号处理的相关配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td> front.aecBinPath </td>
	<td> string</td>
	<td>aec 算法资源路径</td>
	<td>可选</td>
</tr>

<tr>
	<td> front.wakeupBinPath </td>
	<td>string</td>
	<td>唤醒算法资源路径</td>
	<td>必选</td>
</tr>

<tr>
	<td> front.beamformingBinPath </td>
	<td>string</td>
	<td>beamforming 算法资源路径</td>
	<td>可选</td>
</tr>

<tr>
	<td> vad </td>
	<td> json 对象 </td>
	<td>vad 算法模块配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td> vad.resBinPath </td>
	<td>string</td>
	<td>vad算法资源路径 </td>
	<td>必选</td>
</tr>

<tr>
	<td> vad.pauseTime </td>
	<td>int</td>
	<td>vad截止检测时长，单位为 ms，默认为 500ms  </td>
	<td>可选</td>
</tr>

<tr>
	<td> vad.slienceTimeout </td>
	<td>int</td>
	<td> vad 的静音检测超时时长，单位为s，默认为 6s </td>
	<td>可选</td>
</tr>

<tr>
	<td> cloud </td>
	<td>json 对象 </td>
	<td>云端产品的相关配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td>cloud.productId </td>
	<td>string</td>
	<td>dui平台上创建的产品ID</td>
	<td>必选 </td>
</tr>

<tr>
	<td> cloud.aliasKey </td>
	<td>string</td>
	<td> dui平台上创建的产品ID 发布支持， 取值为 "prod | test"</td>
	<td>可选，默认为 prod 分支</td>
</tr>

<tr>
	<td>recorder </td>
	<td>json 对象 </td>
	<td>录音机的相关配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td> recorder.mode </td>
	<td> string </td>
	<td> 录音方式，取值为 "internal | external" 分列表示内部录音和外部录音。  </td>
	<td> 可选 </td>
</tr>

<tr>
	<td> recorder.samplerate </td>
	<td> int </td>
	<td> 录音采样率 </td>
	<td> 必选 </td>
</tr>

<tr>
	<td> recorder.bits </td>
	<td>int</td>
	<td>录音采样位数</td>
	<td>必选</td>
</tr>

<tr>
	<td> recorder.channels </td>
	<td>int</td>
	<td>录音采用通道数</td>
	<td>必选</td>
</tr>

<tr>
	<td> recorder.device </td>
	<td> string </td>
	<td>内部录音机的设备名，默认当前系统的 default 音频设备 </td>
	<td>可选</td>
</tr>

<tr>
	<td> player</td>
	<td> json 对象 </td>
	<td>内部播放器的设置 </td>
	<td>可选</td>
</tr>

<tr>
	<td> player.device </td>
	<td> string </td>
	<td>内部播放器的设备名，默认为 default </td>
	<td>可选</td>
</tr>

<tr>
	<td> tts </td>
	<td>json 对象</td>
	<td>合成音的相关配置</td>
	<td>必选</td>
</tr>


<tr>
	<td> tts.type </td>
	<td>string </td>
	<td>合成音的类型，支持 "cloud" | "local" 分别表示云端合成和本地合成</td>
	<td>必选</td>
</tr>
<tr>
	<td> tts.voice </td>
	<td>string </td>
	<td>合成音的音色，如果为本地合成， 仅支持 "zhilingf" </td>
	<td>必选</td>
</tr>

<tr>
	<td> tts.zhilingf </td>
	<td> json 对象 </td>
	<td>当 tts.type 为 "local"时，会根据 tts.voice 选择对应音色的合成资源路径。</td>
	<td>可选</td>
</tr>

<tr>
	<td> tts.zhilingf.resBinPath </td>
	<td> string </td>
	<td>本地合成 zhilingf 的资源路径</td>
	<td>可选</td>
</tr>

<tr>
	<td> tts.zhilingf.dictPath </td>
	<td> string </td>
	<td>本地合成 zhilingf 的词典路径</td>
	<td>可选</td>
</tr>

<tr>
	<td> tts.volume </td>
	<td> int </td>
	<td>合成音的音量 </td>
	<td>可选</td>
</tr>

<tr>
	<td> tts.speed </td>
	<td> int </td>
	<td>合成音的速度 </td>
	<td>可选</td>
</tr>

<tr>
	<td> oneShot </td>
	<td> json 对象 </td>
	<td> oneshot 模块配置 </td>
	<td>必选</td>
</tr>

<tr>
	<td> oneShot.enable </td>
	<td> bool </td>
	<td> 是否启用oneshot，当前仅支持 false </td>
	<td>可选</td>
</tr>

<tr>
	<td> abnormal </td>
	<td> json 对象 </td>
	<td> sdk异常情况下对话配置 </td>
	<td>可选</td>
</tr>

<tr>
	<td> abnormal.netErrorHint </td>
	<td> string </td>
	<td> 网络错误下的提示音，需要配置成本地文件，网络不好的情况下云端合成也用不了。 </td>
	<td>可选</td>
</tr>

<tr>
	<td> abnormal.ttsErrorHint </td>
	<td> string </td>
	<td> 云端tts合成播放错误情况下的的提示音，需要配置成本地文件。 </td>
	<td>可选</td>
</tr>

<tr>
	<td> debug</td>
	<td> json 对象 </td>
	<td> 保存音频的配置选项 </td>
	<td>可选</td>
</tr>
<tr>
	<td> debug.recAudioDumpFile</td>
	<td> string </td>
	<td> 原始录音保存文件路径 </td>
	<td>可选</td>
</tr>
<tr>
	<td> debug.bfAudioDumpFile</td>
	<td> string </td>
	<td> beamforming算法输出的音频文件路径 </td>
	<td>可选</td>
</tr>

</table>


**唤醒词说明**

```
{
           "greetingFile":"path:./res/tts/help.mp3",
			"greeting": "我在，有什么可以帮你",
			"pinyin": "ni hao xiao chi",
			"name": "你好小驰",
			"threshold": 0.127
		}
		

greetingFile: 唤醒之后播放的提示音，支持本地录音文件，传入文件路径。 
greeting: 唤醒之后的提示文本， sdk内部合成。
pinyin: 唤醒词拼音。
name: 唤醒词的中文。
threshold: 唤醒词阈值。

唤醒提示音播放优先级: 如果配置了 greetingFile 则播放 greetingFile ，
否则播放 greeting 。

```


**命令唤醒词说明**

```
{
			"pinyin": "jiang di yin liang",
			"threshold": 0.100,
			"action": "decrease.volume",
			"name": "降低音量"
		}
		
pinyin: 唤醒词拼音。
name: 唤醒词的中文。
threshold: 唤醒词阈值。
action: 该命令唤醒词对应的动作，比如这个例子中，sdk回调函数会抛出  
command://decrease.volume 消息。 
```


**用户事件说明**

用户可以通过dds_client_publish(struct dds_client *ds, int ev, const char *data);    接口给sdk发送事件。

`DDS_CLIENT_USER_EXTERNAL_WAKEUP` 表示外部唤醒事件，用户可以在收到声纹计算结果之后通过这个事件让sdk继续交互。

这个事件传入的数据格式为：

{
	"nlg":"xxxx"
}

可以通过nlg字段传入所有播报的内容，播放完成之后就可以做识别了。　如果传入NULL表示立刻开始识别。


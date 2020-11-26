
### The Noja programming language

This is the implementation of a programming language I came up with called Noja. 
Noja is the misspelling of the italian word for boredom "noia". 

This is what it looks like:

```js
import "samples/imported.noja" as imp;

print(imp->some_text);

print(imp->list);

item = imp->list->head;
i = 0;

while i < imp->list->count {

	

	print("Item is ", item);

	item = item->next;

	i = i + 1;
}

while 1 {
	
	print("Hello!");

	break;
	continue;
}

a = [];

function(a) {

	a[0] = 0;
	a[1] = 0;

}(a);

print("a = ", a);

{
	test = 10;
}

print("test is equal to ", test);
```

I wrote it as an exercise so it's not innovative in any way. The internal implementation looks very much like CPython's since it's been my main reference.
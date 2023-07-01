1. Clean main.cpp: - not vital

'glTF' should be only used by a higher class which will search a 'models' dir for glTF 
models, load them, and then serialize them. Once a model is loaded and serialized, it should be written to a 
file in its serialized state, so models do not have to re-serialized each runtime - the serialized model is loaded 
instead. Hashes of the models should be stored alongside model names: if the hash for a model changes, it is 
re-serialized and rehashed; if a new name is found, it is serialized and hashed.

2. Actually use serialized glTF - big job

Create the infrastructure required for actaully drawing models. Need functions and objects to create the 
necessary resources and submit draw commands. (This will also be optimised, but dont go straight to complete shit 
version...)

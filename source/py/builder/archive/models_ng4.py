"""model: schema of builder

Baically a simplified copy of the xcode model. The only difference is that I have
renamed 'Target' as 'Builder' and 'Workspace' as 'Recipe' since it
makes more sense in this context.

Recipe -- Settings
    |           ∆
    |*          |
    Project -- Settings
        |           ∆
        |*          |
        Builder -- Settings
            |
            v
            Product

"""
import pathlib
from abc import ABC, abstractmethod
from types import SimpleNamespace


class Settings(SimpleNamespace):
    """A dictionary object with dotted access to its members.

    >>> settings = Settings(**dict)
    """

    def copy(self):
        """provide a copy of the internal dictionary"""
        return Settings(**self.__dict__.copy())

    def update(self, other):
        """Like a dict.update but using the internal dict instead"""
        if isinstance(other, dict):
            self.__dict__.update(other)
        elif isinstance(other, Settings):
            self.__dict__.update(other.__dict__)
        else:
            raise TypeError


class Product:
    """Produced by running a builder."""

    def __init__(self, name: str, path: pathlib.Path):
        self.name = name
        self.path = pathlib.Path(path)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}'>"

    def exists(self) -> bool:
        """returns True if product exists at self.path"""
        return self.path.exists()


class Builder:
    """A Builder know how to build a single product type in a project.

    A Builder is analagous to a Target in Xcode.
    """

    def __init__(self, name: str = None, **settings):
        self.name = name
        self.product = None
        self.settings = Settings(**settings)
        # self.settings = Settings(**settings)

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    def build(self):
        """build product"""
        print(f"\t\t{self} -> {self.product}")


class Project:
    """A repository for all the files, resources, and information required to
    build one or more software products.
    """

    def __init__(self, name: str = None , builders: list[Builder] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.builders = builders

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    @classmethod
    def from_factory(cls, name=None, builder_classes=None, **settings):
        if not builder_classes:
            builder_classes = cls.builder_classes
        _builders = []
        for builder_class in builder_classes:
            builder = builder_class(**settings)
            _builders.append(builder)
        return cls(name, _builders, **settings)

    def build(self):
        """sequence builders in order of dependency"""
        print(f"\t{self}")
        for builder in self.builders:
            builder.build()


class Recipe:
    """A platform-specific container for multiple build projects.

    A recipe is analogous to a workspace in xcode
    """

    def __init__(self, name: str, projects: list[Project] = None, **settings):
        self.name = name
        self.settings = Settings(**settings)
        self.projects = projects

    def __str__(self):
        return f"<{self.__class__.__name__}:'{self.name}-{self.settings}'>"

    __repr__=__str__

    @classmethod
    def from_factory(cls, name=None, project_classes=None, **settings):
        if not project_classes:
            project_classes = cls.project_classes
        _projects = []
        for project_class in project_classes:
            _builders = []
            for builder_class in project_class.builder_classes:
                builder = builder_class(**settings)
                _builders.append(builder)
            project = project_class(name=None, builders=_builders, **settings)
            _projects.append(project)
        return cls(name, _projects, **settings)

    def build(self):
        """build projects in order of dependency"""
        print(f"{self}")
        for project in self.projects:
            project.build()




def test():
    """testing the schema"""

    # product classes
    class PyExt(Product):
        name = "py.mxo"

    class PyJSExt(Product):
        name = "pyjs.mxo"

    class StaticPython(Product):
        name = "static-python"


    # builder classes
    class StaticPythonBuilder(Builder):
        product_class = StaticPython

    class PyExtBuilder(Builder):
        product_class = PyExt

    class PyJSExtBuilder(Builder):
        product_class = PyJSExt


    # project classes
    class StaticPythonProject(Project):
        builder_classes = [StaticPythonBuilder]

    class PyJSExternalsProject(Project):
        builder_classes = [PyExtBuilder, PyJSExtBuilder]

    # recipe classes
    class StaticPyJSRecipe(Recipe):
        project_classes = [StaticPythonProject, PyJSExternalsProject]


    # recipe / workspace
    r1 = StaticPyJSRecipe.from_factory("static-py-js-recipe", a=1, b=2, x=2000)
    r1.build()



if __name__ == "__main__":
    test()

# Exercício 001 - Construção de diagrama de classes

Aluno: Aline Barbosa Vidal
RA: 10721348


Neste exercício construiremos um diagrama de classes utilizando o PlantUML.


## Diagrama

Desenhe aqui o seu diagrama.

@startuml DiagramaClasses_Reduzido

class Curso {
  - nome: String
  - grau: String
  - modalidade: String
  - cargaHorariaTotal: int
  + listarComponentes(): List<ComponenteCurricular>
}

class ComponenteCurricular {
  - codigo: String
  - modalidade: String
  - creditos: int
  - cargaHoraria: int
  - periodo: String
  + getTurmasOfertadas(): List<Turma>
}

class Turma {
  - codigo: String
  - semestre: String
  - vagas: int
  - horario: String
  + adicionarAluno(aluno: Aluno): void
  + removerAluno(aluno: Aluno): void
}

class Professor {
  - nome: String
  - registroAcademico: String
  - titulacao: String
  + ministrarTurma(turma: Turma): void
  + fazerChamada(turma: Turma): void
  + atribuirNota(aluno: Aluno, avaliacao: String, nota: double): void
}

class Aluno {
  - nome: String
  - registroAcademico: String
  - email: String
  - situacao: String
  - media: double
  + matricularEm(turma: Turma): void
  + trancarDisciplina(turma: Turma): void
}

' ===================== ASSOCIAÇÕES =====================

Curso "1" -- "1..*" ComponenteCurricular : Possui >

ComponenteCurricular "1" -- "0..*" Turma : Composto Por >

Curso "1..*" -- "0..*" Professor : Leciona em <

Professor "1" -- "0..*" Turma : Ministra >

Turma "0..*" -- "0..*" Aluno : Matriculado em

@enduml

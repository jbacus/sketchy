#!/usr/bin/env python3
"""
PM-Agent (Project Manager Agent)
Orchestrates the multi-agent workflow for the Sketchy 3D Modeler project.
"""

import json
import os
from dataclasses import dataclass
from typing import List, Optional, Dict
from enum import Enum


class TaskStatus(Enum):
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    BLOCKED = "blocked"
    COMPLETED = "completed"
    FAILED = "failed"


@dataclass
class TaskHandoff:
    """Standardized task handoff message for agent delegation."""
    task_id: str
    phase: int
    agent_name: str
    goal: str
    acceptance_criteria: List[str]
    code_location: str
    dependencies: List[str]
    dependency_status: str

    def to_json(self) -> str:
        return json.dumps(self.__dict__, indent=2)


class PMAgent:
    """
    Project Manager Agent - The Orchestrator

    Responsibilities:
    - Global state management
    - Task decomposition (Phase -> Task -> Subtask)
    - Scheduling and delegation
    - Quality gate approval
    - Budget tracking (token usage)
    """

    def __init__(self, roadmap_path: str, registry_path: str):
        self.roadmap_path = roadmap_path
        self.registry_path = registry_path
        self.roadmap = self._load_roadmap()
        self.agent_registry = self._load_agent_registry()
        self.task_queue: List[TaskHandoff] = []

    def _load_roadmap(self) -> Dict:
        """Load the project roadmap from JSON."""
        with open(self.roadmap_path, 'r') as f:
            return json.load(f)

    def _load_agent_registry(self) -> List[Dict]:
        """Load the agent registry from JSON."""
        with open(self.registry_path, 'r') as f:
            return json.load(f)

    def save_roadmap(self):
        """Persist roadmap changes to disk."""
        with open(self.roadmap_path, 'w') as f:
            json.dump(self.roadmap, f, indent=2)

    def get_current_phase(self) -> Optional[Dict]:
        """Get the currently active phase."""
        for phase in self.roadmap['phases']:
            if phase['status'] in ['in_progress', 'pending']:
                return phase
        return None

    def get_next_task(self) -> Optional[Dict]:
        """
        Get the next available task based on dependencies.
        Returns a task that has all dependencies satisfied.
        """
        current_phase = self.get_current_phase()
        if not current_phase:
            return None

        for task in current_phase['tasks']:
            if task['status'] == 'pending':
                # Check if dependencies are satisfied
                if self._check_dependencies(task):
                    return task

        return None

    def _check_dependencies(self, task: Dict) -> bool:
        """Check if all task dependencies are completed."""
        if not task.get('dependencies'):
            return True

        # Search all phases for dependency tasks
        for phase in self.roadmap['phases']:
            for t in phase['tasks']:
                if t['task_id'] in task['dependencies']:
                    if t['status'] != 'completed':
                        return False
        return True

    def delegate_task(self, task: Dict) -> TaskHandoff:
        """
        Create a TaskHandoff message and delegate to the appropriate agent.
        """
        handoff = TaskHandoff(
            task_id=task['task_id'],
            phase=self._get_phase_for_task(task['task_id']),
            agent_name=task['assigned_agent'],
            goal=task['name'],
            acceptance_criteria=task.get('acceptance_criteria', []),
            code_location=self._get_code_location(task),
            dependencies=task.get('dependencies', []),
            dependency_status="satisfied" if self._check_dependencies(task) else "pending"
        )

        self.task_queue.append(handoff)
        return handoff

    def _get_phase_for_task(self, task_id: str) -> int:
        """Extract phase number from task ID."""
        return int(task_id.split('-')[0][1:])

    def _get_code_location(self, task: Dict) -> str:
        """Determine the code location for a task."""
        agent_name = task['assigned_agent']

        # Map agents to code directories
        location_map = {
            'Kernel-Agent': 'src/kernel/',
            'Graphics-Agent': 'src/graphics/',
            'Modeling-Agent': 'src/modeling/',
            'IO-Agent': 'src/io/',
            'UXUI-Agent': 'src/ui/',
            'QA-Agent': 'tests/',
        }

        return location_map.get(agent_name, 'src/')

    def update_task_status(self, task_id: str, status: str):
        """Update the status of a specific task."""
        for phase in self.roadmap['phases']:
            for task in phase['tasks']:
                if task['task_id'] == task_id:
                    task['status'] = status
                    self.save_roadmap()
                    return

    def generate_status_report(self) -> str:
        """Generate a human-readable status report."""
        report = f"=== {self.roadmap['project_name']} Status Report ===\n\n"

        for phase in self.roadmap['phases']:
            report += f"Phase {phase['phase_id']}: {phase['name']} [{phase['status'].upper()}]\n"

            for task in phase['tasks']:
                status_icon = {
                    'pending': '⏳',
                    'in_progress': '🔄',
                    'completed': '✅',
                    'blocked': '🚫',
                    'failed': '❌'
                }.get(task['status'], '❓')

                report += f"  {status_icon} {task['task_id']}: {task['name']}\n"
                report += f"     Agent: {task['assigned_agent']}\n"

            report += "\n"

        return report


if __name__ == "__main__":
    # Initialize PM-Agent
    pm = PMAgent(
        roadmap_path="../roadmap.json",
        registry_path="../agent_registry.json"
    )

    print(pm.generate_status_report())

    # Get next task
    next_task = pm.get_next_task()
    if next_task:
        print(f"\n📋 Next Task Ready:")
        print(f"   {next_task['task_id']}: {next_task['name']}")
        print(f"   Assigned to: {next_task['assigned_agent']}")

        # Create handoff message
        handoff = pm.delegate_task(next_task)
        print(f"\n📨 Task Handoff Message:\n{handoff.to_json()}")
